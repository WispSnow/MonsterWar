#include "selection_system.h"
#include "../component/player_component.h"
#include "../component/enemy_component.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/component/transform_component.h"
#include "../defs/constants.h"
#include "../defs/tags.h"
#include "../../engine/utils/math.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/sigh.hpp>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace game::system {

SelectionSystem::SelectionSystem(entt::registry& registry, engine::core::Context& context)
    : registry_(registry), context_(context) {
    context_.getInputManager().onAction("mouse_left"_hs).connect<&SelectionSystem::onMouseLeftClick>(this);
    context_.getInputManager().onAction("mouse_right"_hs).connect<&SelectionSystem::onMouseRightClick>(this);
}

SelectionSystem::~SelectionSystem() {
    context_.getInputManager().onAction("mouse_left"_hs).disconnect<&SelectionSystem::onMouseLeftClick>(this);
    context_.getInputManager().onAction("mouse_right"_hs).disconnect<&SelectionSystem::onMouseRightClick>(this);
}

void SelectionSystem::update() {
    auto mouse_pos = context_.getInputManager().getLogicalMousePosition();
    // 优先判断玩家单位
    auto view_player = registry_.view<engine::component::TransformComponent, game::component::PlayerComponent>();
    for (auto entity : view_player) {
        auto& transform = view_player.get<engine::component::TransformComponent>(entity);
        // 判断是否在鼠标悬浮检测范围内
        if (engine::utils::distanceSquared(transform.position_, mouse_pos) <= defs::HOVER_RADIUS * defs::HOVER_RADIUS) {
            registry_.ctx().get<entt::entity&>("hovered_unit"_hs) = entity;
            return;   // 找到悬浮单位，直接返回
        }
    }
    // 如果玩家单位没有被选中，再判断敌方单位
    auto view_enemy = registry_.view<engine::component::TransformComponent, game::component::EnemyComponent>();
    for (auto entity : view_enemy) {
        auto& transform = view_enemy.get<engine::component::TransformComponent>(entity);
        if (engine::utils::distanceSquared(transform.position_, mouse_pos) <= defs::HOVER_RADIUS * defs::HOVER_RADIUS) {
            registry_.ctx().get<entt::entity&>("hovered_unit"_hs) = entity;
            return;
        }
    }
    // 如果都没有被悬浮，则不悬浮任何单位
    registry_.ctx().get<entt::entity&>("hovered_unit"_hs) = entt::null;
}

void SelectionSystem::clearCurrentSelection() {
    auto current_selected_unit = registry_.ctx().get<entt::entity&>("selected_unit"_hs);
    // 移除之前选中的单位，并移除范围显示标签
    if (current_selected_unit != entt::null && registry_.valid(current_selected_unit)) {
        registry_.remove<game::defs::ShowRangeTag>(current_selected_unit);
    }
    registry_.ctx().get<entt::entity&>("selected_unit"_hs) = entt::null;
}

// --- 输入控制回调函数 ---
bool SelectionSystem::onMouseLeftClick() {
    auto hovered_unit = registry_.ctx().get<entt::entity&>("hovered_unit"_hs);
    if (hovered_unit == entt::null || !registry_.valid(hovered_unit)) return false;
    // 如果鼠标悬浮单位是玩家，则选中单位，并清除之前选中的单位
    if (auto player = registry_.try_get<game::component::PlayerComponent>(hovered_unit); player) {
        clearCurrentSelection();
        registry_.ctx().get<entt::entity&>("selected_unit"_hs) = hovered_unit;
        // 添加范围显示标签
        registry_.emplace_or_replace<game::defs::ShowRangeTag>(hovered_unit);
        return true;
    }
    return false;
}

bool SelectionSystem::onMouseRightClick() {
    clearCurrentSelection();
    return false;   // 让鼠标右键可以穿透
}

} // namespace game::system