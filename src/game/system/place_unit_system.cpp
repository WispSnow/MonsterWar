#include "place_unit_system.h"
#include "../data/session_data.h"
#include "../data/game_stats.h"
#include "../defs/events.h"
#include "../defs/tags.h"
#include "../component/place_occupied_component.h"
#include "../factory/entity_factory.h"
#include "../component/unit_prep_component.h"
#include "../../engine/core/context.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/render/camera.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/name_component.h"
#include "../../engine/component/render_component.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::system {

PlaceUnitSystem::PlaceUnitSystem(entt::registry& registry, 
    game::factory::EntityFactory& entity_factory,
    engine::core::Context& context)
    : registry_(registry), entity_factory_(entity_factory), context_(context) {
    // 注册按键
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).connect<&PlaceUnitSystem::onCancelPrepUnit>(this);
    input_manager.onAction("mouse_left"_hs).connect<&PlaceUnitSystem::onPlaceUnit>(this);
    // 注册事件
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<game::defs::PrepUnitEvent>().connect<&PlaceUnitSystem::onPrepUnitEvent>(this);
    dispatcher.sink<game::defs::RemovePlayerUnitEvent>().connect<&PlaceUnitSystem::onRemoveUnitEvent>(this);
}

PlaceUnitSystem::~PlaceUnitSystem() {
    // 断开按键
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).disconnect<&PlaceUnitSystem::onCancelPrepUnit>(this);
    input_manager.onAction("mouse_left"_hs).disconnect<&PlaceUnitSystem::onPlaceUnit>(this);
    // 断开事件
    context_.getDispatcher().disconnect(this);
}

void PlaceUnitSystem::update(float) {
    // 目标放置位置先置为null，只有找到了有效位置才会被赋值
    target_place_entity_ = entt::null;

    auto view = registry_.view<game::component::UnitPrepComponent, engine::component::TransformComponent>();
    // 虽然是循环，但拥有UnitPrepComponent的实体最多只有一个
    for (auto entity : view) {
        // 位置同步到到鼠标
        const auto& mouse_pos_screen = context_.getInputManager().getLogicalMousePosition();
        const auto mouse_pos_world = context_.getCamera().screenToWorld(mouse_pos_screen);
        auto& transform = view.get<engine::component::TransformComponent>(entity);
        transform.position_ = mouse_pos_world;

        // 检查放置位置是否有效
        const auto& unit_prep = view.get<game::component::UnitPrepComponent>(entity);
        checkTargetPlace(transform.position_, unit_prep.type_);
        
        // 根据是否有效设置颜色
        auto& render = registry_.get<engine::component::RenderComponent>(entity);
        target_place_entity_ != entt::null ? render.color_ = engine::utils::FColor::green() 
                                           : render.color_ = engine::utils::FColor::red();
    }
}

void PlaceUnitSystem::checkTargetPlace(const glm::vec2& position, game::defs::PlayerType player_type) {
    // 检查是否处在近战可放置区域（拥有MeleePlaceTag的地点）
    if (player_type == game::defs::PlayerType::MELEE) {
        auto melee_place_view = registry_.view<game::defs::MeleePlaceTag, 
            engine::component::TransformComponent, 
            engine::component::SpriteComponent>(entt::exclude<game::component::PlaceOccupiedComponent>);
        for (auto place_entity : melee_place_view) {
            auto& place_transform = melee_place_view.get<engine::component::TransformComponent>(place_entity);
            auto& place_sprite = melee_place_view.get<engine::component::SpriteComponent>(place_entity);
            // 检测与放置区域中心的距离（Tiled中的参照点是左上角）
            auto center_position = place_transform.position_ + place_sprite.size_ * place_transform.scale_ / 2.0f;
            if (engine::utils::distanceSquared(position, center_position) < game::defs::PLACE_RADIUS * game::defs::PLACE_RADIUS) {
                target_place_entity_ = place_entity;
                return;
            }
        }
    // 检查是否处在远程可放置区域（拥有RangedPlaceTag的地点）
    } else if (player_type == game::defs::PlayerType::RANGED) {
        auto ranged_place_view = registry_.view<game::defs::RangedPlaceTag, 
            engine::component::TransformComponent, 
            engine::component::SpriteComponent>(entt::exclude<game::component::PlaceOccupiedComponent>);
        for (auto place_entity : ranged_place_view) {
            auto& place_transform = ranged_place_view.get<engine::component::TransformComponent>(place_entity);
            auto& place_sprite = ranged_place_view.get<engine::component::SpriteComponent>(place_entity);
            auto center_position = place_transform.position_ + place_sprite.size_ * place_transform.scale_ / 2.0f;
            if (engine::utils::distanceSquared(position, center_position) < game::defs::PLACE_RADIUS * game::defs::PLACE_RADIUS) {
                target_place_entity_ = place_entity;
                return;
            }
        }
    }
}

void PlaceUnitSystem::onPrepUnitEvent(const game::defs::PrepUnitEvent& event) {
    // 如果cost资源不够，直接返回
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    if (game_stats.cost_ < event.cost_) return;

    // 先移除其他单位准备类型实体
    onCancelPrepUnit();

    // 在鼠标所在的位置创建单位准备类型实体
    auto screen_position = context_.getInputManager().getLogicalMousePosition();
    auto position = context_.getCamera().screenToWorld(screen_position);
    entity_factory_.createUnitPrep(event.name_id_, event.class_id_, event.cost_, position);
    spdlog::info("创建单位准备类型实体: {}, pos: {}, {}", event.name_id_, position.x, position.y);
}

void PlaceUnitSystem::onRemoveUnitEvent(const game::defs::RemovePlayerUnitEvent& event) {
    // 标记该单位为死亡
    registry_.emplace_or_replace<game::defs::DeadTag>(event.entity_);
    // 检查所有被占用的地点，如果占用者是移除事件中的单位，则移除占用组件
    auto view = registry_.view<game::component::PlaceOccupiedComponent>();
    for (auto entity : view) {
        auto& place_occupied = view.get<game::component::PlaceOccupiedComponent>(entity);
        if (place_occupied.entity_ == event.entity_) {
            registry_.remove<game::component::PlaceOccupiedComponent>(entity);
            spdlog::info("移除地点 {} 的占用组件", entt::to_integral(entity));
            break;
        }
    }
}

bool PlaceUnitSystem::onPlaceUnit() {
    // 目标放置位置有效才继续
    if (target_place_entity_ == entt::null) return false;

    // 获取目标位置坐标
    const auto& transform = registry_.get<engine::component::TransformComponent>(target_place_entity_);
    const auto& sprite = registry_.get<engine::component::SpriteComponent>(target_place_entity_);
    auto position = transform.position_ + sprite.size_ * transform.scale_ / 2.0f;
    // 获取单位信息
    auto unit_map = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>()->getUnitMap();
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    auto view_prep = registry_.view<game::component::UnitPrepComponent>();
    // 循环只会进行一次，因为拥有UnitPrepComponent的实体最多只有一个
    for (auto entity : view_prep) {
        const auto& unit_prep_component = registry_.get<game::component::UnitPrepComponent>(entity);
        auto& unit_data = unit_map[unit_prep_component.name_id_];
        // 创建单位
        auto unit_entity = entity_factory_.createPlayerUnit(unit_data.class_id_, position, unit_data.level_, unit_data.rarity_);
        registry_.emplace<engine::component::NameComponent>(unit_entity, unit_data.name_id_, unit_data.name_);
        // 地点实体添加占用组件
        registry_.emplace<game::component::PlaceOccupiedComponent>(target_place_entity_, unit_entity);
        // 扣除费用
        game_stats.cost_ -= unit_prep_component.cost_;
        // 移除单位准备类型实体
        registry_.emplace_or_replace<game::defs::DeadTag>(entity);

        // 通知UI移除对应肖像
        context_.getDispatcher().enqueue(game::defs::RemoveUIPortraitEvent{unit_data.name_id_});

        // --- 渲染图层修正：确保玩家所在图层大于放置点图标的图层 ---
        const auto& render_place = registry_.get<engine::component::RenderComponent>(target_place_entity_);
        // 正常情况下render_place.layer应该不会超过主图层（10），那么不做处理
        // 如果超过了，就让玩家所在图层 = 放置点图层 + 1
        if (render_place.layer > engine::component::RenderComponent::MAIN_LAYER) {
            auto& render_player = registry_.get<engine::component::RenderComponent>(unit_entity);
            render_player.layer = render_place.layer + 1;
        }
        // 如果拥有被动技能，则立刻释放技能
        if (registry_.all_of<game::defs::PassiveSkillTag>(unit_entity)) {
            context_.getDispatcher().enqueue(game::defs::SkillActiveEvent{unit_entity});
        }

    }
    // 播放放置音效
    context_.getAudioPlayer().playSound("unit_placed"_hs);
    return true;
}

bool PlaceUnitSystem::onCancelPrepUnit() {
    // 移除所有单位准备类型实体
    auto view = registry_.view<game::component::UnitPrepComponent>();
    for (auto entity : view) {
        registry_.emplace_or_replace<game::defs::DeadTag>(entity);
        spdlog::info("移除单位准备类型实体: {}", entt::to_integral(entity));
    }
    return false;   // 让鼠标右键可以穿透
}

}   // namespace game::system