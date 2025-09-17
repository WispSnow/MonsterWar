#pragma once
#include "../data/entity_blueprint.h"
#include <entt/entity/fwd.hpp>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace game::factory {

class BlueprintManager;
/**
 * @brief 实体工厂，用于创建不同类型的实体
 * 
 * 实体工厂通过蓝图管理器获取蓝图数据，并创建不同类型的实体。
 */
class EntityFactory {
private:
    entt::registry& registry_;
    BlueprintManager& blueprint_manager_;

public:
    /// @brief 实体工厂构造函数, 需要传入注册表和蓝图管理器。通过蓝图数据创建不同实体
    EntityFactory(entt::registry& registry, BlueprintManager& blueprint_manager);

    entt::entity createPlayerUnit(entt::id_type class_id, const glm::vec2& position, int level = 1, int rarity = 1);
    entt::entity createEnemyUnit(entt::id_type class_id, const glm::vec2& position, int target_waypoint_id, int level = 1, int rarity = 1);
    entt::entity createProjectile(entt::id_type id, const glm::vec2& start_position, const glm::vec2& target_position, entt::entity target, float damage);
    // TODO: 未来添加其他实体的创建函数

private:
    // --- 组件创建函数 ---
    void addTransformComponent(entt::entity entity, const glm::vec2& position, const glm::vec2& scale = glm::vec2(1.0f), float rotation = 0.0f);
    void addSpriteComponent(entt::entity entity, const data::SpriteBlueprint& sprite, const bool is_flipped = false);
    void addAnimationComponent(entt::entity entity, 
        const std::unordered_map<entt::id_type, data::AnimationBlueprint>& animation_blueprints, 
        const data::SpriteBlueprint& sprite_blueprint,
        entt::id_type default_animation_id);
    void addStatsComponent(entt::entity entity, const data::StatsBlueprint& stats, int level = 1, int rarity = 1);
    void addPlayerComponent(entt::entity entity, const data::PlayerBlueprint& player, int rarity);
    void addEnemyComponent(entt::entity entity, const data::EnemyBlueprint& enemy, int target_waypoint_id);
    void addAudioComponent(entt::entity entity, const data::SoundBlueprint& sounds);
    void addProjectileIDComponent(entt::entity entity, entt::id_type id);
    // TODO: 未来添加其他组件创建函数
};

}   // namespace game::factory