#include "entity_factory.h"
#include "blueprint_manager.h"
#include "../data/entity_blueprint.h"
#include "../../engine/utils/math.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/render_component.h"
#include "../defs/tags.h"
#include "../../engine/component/audio_component.h"
#include "../component/stats_component.h"
#include "../component/enemy_component.h"
#include "../component/class_name_component.h"
#include "../component/player_component.h"
#include "../component/blocker_component.h"
#include "../component/projectile_component.h"
#include <entt/entity/registry.hpp>
#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

using namespace entt::literals;

namespace game::factory {

EntityFactory::EntityFactory(entt::registry& registry, 
    BlueprintManager& blueprint_manager)
    : registry_(registry), blueprint_manager_(blueprint_manager) {}

    entt::entity EntityFactory::createPlayerUnit(entt::id_type class_id, const glm::vec2& position, int level, int rarity) {
        auto entity = registry_.create();
        const auto& blueprint = blueprint_manager_.getPlayerClassBlueprint(class_id);
        // --- 添加组件 ---
        // 添加Transform组件
        addTransformComponent(entity, position);
    
        // 添加Sprite组件
        addSpriteComponent(entity, blueprint.sprite_);
    
        // 添加Animation组件
        addAnimationComponent(entity, blueprint.animations_, blueprint.sprite_, "idle"_hs);
    
        // 添加Audio组件
        addAudioComponent(entity, blueprint.sounds_);
    
        // 添加Stats组件
        addStatsComponent(entity, blueprint.stats_, level, rarity);
        
        // 添加Player组件
        addPlayerComponent(entity, blueprint.player_, rarity);

        // 添加ProjectileID组件
        addProjectileIDComponent(entity, blueprint.projectile_id_);
    
        // 补充其他必要组件
        registry_.emplace<game::component::ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);
        registry_.emplace<engine::component::RenderComponent>(entity);
        
        // 未来可添加其它组件
    
        return entity;
    }

entt::entity EntityFactory::createEnemyUnit(entt::id_type class_id, const glm::vec2& position, int target_waypoint_id, int level, int rarity) {
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getEnemyClassBlueprint(class_id);
    // --- 添加组件 ---
    // 添加Transform组件
    addTransformComponent(entity, position);

    // 添加Sprite组件
    addSpriteComponent(entity, blueprint.sprite_);

    // 添加Animation组件 (默认动画为“walk”)
    addAnimationComponent(entity, blueprint.animations_, blueprint.sprite_, "walk"_hs);

    // 添加Audio组件
    addAudioComponent(entity, blueprint.sounds_);

    // 添加Stats组件
    addStatsComponent(entity, blueprint.stats_, level, rarity);
    
    // 添加Enemy组件
    addEnemyComponent(entity, blueprint.enemy_, target_waypoint_id);

    // 添加ProjectileID组件
    addProjectileIDComponent(entity, blueprint.projectile_id_);

    // 补充其他必要组件
    registry_.emplace<game::component::ClassNameComponent>(entity, class_id, blueprint.display_info_.name_);
    registry_.emplace<engine::component::RenderComponent>(entity);  // 使用默认主图层
    
    // 未来可添加其它组件

    return entity;
}

entt::entity EntityFactory::createProjectile(entt::id_type id, const glm::vec2& start_position, const glm::vec2& target_position, entt::entity target, float damage) {
    // 创建投射物实体
    auto entity = registry_.create();
    const auto& blueprint = blueprint_manager_.getProjectileBlueprint(id);
    // --- 依次添加必要组件 ---
    // 添加ProjectileComponent
    registry_.emplace<game::component::ProjectileComponent>(entity, 
        target, 
        damage,
        start_position,
        target_position, 
        start_position,
        blueprint.arc_height_, 
        blueprint.total_flight_time_,
        0.0f);
    // 添加SpriteComponent
    addSpriteComponent(entity, blueprint.sprite_);
    // 添加TransformComponent
    addTransformComponent(entity, start_position);
    // 添加AudioComponent
    addAudioComponent(entity, blueprint.sounds_);
    // 添加RenderComponent(让投射物位于主图层+1，即可以遮住角色)
    registry_.emplace<engine::component::RenderComponent>(entity, engine::component::RenderComponent::MAIN_LAYER + 1);
    return entity;
}

// --- 组件创建函数 ---

void EntityFactory::addTransformComponent(entt::entity entity, const glm::vec2& position, const glm::vec2& scale, float rotation) {
    registry_.emplace<engine::component::TransformComponent>(entity, position, scale, rotation);
}

void EntityFactory::addSpriteComponent(entt::entity entity, const data::SpriteBlueprint& sprite, const bool is_flipped) {
    registry_.emplace<engine::component::SpriteComponent>(entity, 
        engine::component::Sprite(sprite.path_, 
                                  sprite.src_rect_,
                                  is_flipped),
        sprite.size_,
        sprite.offset_);
    // 如果图片朝左就添加FaceLeftTag
    if (!sprite.face_right_) {
        registry_.emplace<game::defs::FaceLeftTag>(entity);
    }
}

void EntityFactory::addAnimationComponent(entt::entity entity, 
        const std::unordered_map<entt::id_type, data::AnimationBlueprint>& animation_blueprints,
        const data::SpriteBlueprint& sprite_blueprint,
        entt::id_type default_animation_id) {
    // 先创建map容器  
    std::unordered_map<entt::id_type, engine::component::Animation> animations;
    // 针对每一个动画，
    for (const auto& [anim_id, anim_blueprint] : animation_blueprints) {
        // 创建动画帧容器
        std::vector<engine::component::AnimationFrame> frames;
        // 依次读取蓝图中的每一个帧索引
        for (const auto& frame_index : anim_blueprint.frames_) {
            engine::utils::Rect source_rect = sprite_blueprint.src_rect_;
            // 通过索引计算每一帧的源矩形区域
            source_rect.position.x += frame_index * source_rect.size.x;
            source_rect.position.y += anim_blueprint.row_ * source_rect.size.y;
            // 创建动画帧并插入动画帧容器
            frames.emplace_back(source_rect, anim_blueprint.ms_per_frame_);
        }
        // 将创建好的动画帧容器插入动画map容器 (可直接使用蓝图中的事件信息)
        animations.emplace(anim_id, engine::component::Animation(std::move(frames), anim_blueprint.events_));
    }
    // 通过动画map容器创建动画组件
    registry_.emplace<engine::component::AnimationComponent>(entity, std::move(animations), default_animation_id);
}

void EntityFactory::addStatsComponent(entt::entity entity, const data::StatsBlueprint& stats, int level, int rarity) {
    // 计算等级和稀有度对属性的影响 (未来可改成数据驱动方便调整)
    auto hp = engine::utils::statModify(stats.hp_, level, rarity);
    auto atk = engine::utils::statModify(stats.atk_, level, rarity);
    auto def = engine::utils::statModify(stats.def_, level, rarity);
    
    registry_.emplace_or_replace<game::component::StatsComponent>(entity, 
        hp, 
        hp, 
        atk, 
        def, 
        stats.range_,
        stats.atk_interval_,
        0.0f,
        level,
        rarity);
}

void EntityFactory::addPlayerComponent(entt::entity entity, const data::PlayerBlueprint& player, int rarity) {
    auto cost = static_cast<int>(std::round(player.cost_ * (0.9f + 0.1f * rarity)));
    registry_.emplace<game::component::PlayerComponent>(entity, cost);
    // 添加类型标签(近战、远程、治疗)
    if (player.type_ == game::defs::PlayerType::MELEE) {
        registry_.emplace<game::defs::MeleeUnitTag>(entity);    // 近战单位标签
        // 近战类型添加阻挡者组件
        registry_.emplace<game::component::BlockerComponent>(entity, player.block_);
    } else if (player.type_ == game::defs::PlayerType::RANGED) {
        registry_.emplace<game::defs::RangedUnitTag>(entity);    // 远程单位标签
        if (player.healer_) {
            registry_.emplace<game::defs::HealerTag>(entity);    // 治疗单位标签
        }
    }
    // TODO: 未来添加技能组件
}

void EntityFactory::addEnemyComponent(entt::entity entity, const data::EnemyBlueprint& enemy, int target_waypoint_id) {
    registry_.emplace<game::component::EnemyComponent>(entity, target_waypoint_id, enemy.speed_);
    registry_.emplace<engine::component::VelocityComponent>(entity, glm::vec2(0, 0));
    if (enemy.ranged_) {    // 添加远程或近战标签备用
        registry_.emplace<game::defs::RangedUnitTag>(entity);
    } else {
        registry_.emplace<game::defs::MeleeUnitTag>(entity);
    }
}

void EntityFactory::addAudioComponent(entt::entity entity, const data::SoundBlueprint& sounds) {
    if (sounds.sounds_.empty()) return;
    // 将sounds_中的键值对转换为audio_map中的键值对
    std::unordered_map<entt::id_type, entt::id_type> audio_map;
    for (const auto& [sound_key, sound_id] : sounds.sounds_) {
        audio_map.emplace(sound_key, sound_id);
    }
    registry_.emplace<engine::component::AudioComponent>(entity, std::move(audio_map));
}

void EntityFactory::addProjectileIDComponent(entt::entity entity, entt::id_type id) {
    if (id == entt::null) return;
    registry_.emplace<game::component::ProjectileIDComponent>(entity, id);
}

}   // namespace game::factory