#include "timer_system.h"
#include "../component/stats_component.h"
#include "../defs/tags.h"
#include <entt/entity/registry.hpp>

namespace game::system {

void TimerSystem::update(entt::registry& registry, float delta_time) {
    updateAttackTimer(registry, delta_time);
}

void TimerSystem::updateAttackTimer(entt::registry& registry, float delta_time) {
    // 筛选条件：有StatsComponent组件，但没有AttackReadyTag标签（即攻击正在冷却）
    auto view_unit = registry.view<game::component::StatsComponent>(entt::exclude<game::defs::AttackReadyTag>);
    for (auto entity : view_unit) {
        auto& stats = view_unit.get<game::component::StatsComponent>(entity);
        stats.atk_timer_ += delta_time;     // 推进计时器
        // 如果攻击计时器大于等于攻击间隔，代表冷却结束。添加“可攻击”标签，并重置攻击计时器
        if (stats.atk_timer_ >= stats.atk_interval_) {
            registry.emplace_or_replace<game::defs::AttackReadyTag>(entity);
            stats.atk_timer_ = 0.0f;
        }
    }
}

}   // namespace game::system