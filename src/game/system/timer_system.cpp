#include "timer_system.h"
#include "../component/stats_component.h"
#include "../component/skill_component.h"
#include "../defs/tags.h"
#include "../defs/events.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

namespace game::system {

TimerSystem::TimerSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
}

void TimerSystem::update(float delta_time) {
    updateAttackTimer(delta_time);
    updateSkillCooldownTimer(delta_time);
    updateSkillDurationTimer(delta_time);
}

void TimerSystem::updateAttackTimer(float delta_time) {
    // 筛选条件：有StatsComponent组件，但没有AttackReadyTag标签（即攻击正在冷却）
    auto view_unit = registry_.view<game::component::StatsComponent>(entt::exclude<game::defs::AttackReadyTag>);
    for (auto entity : view_unit) {
        auto& stats = view_unit.get<game::component::StatsComponent>(entity);
        stats.atk_timer_ += delta_time;     // 推进计时器
        // 如果攻击计时器大于等于攻击间隔，代表冷却结束。添加“可攻击”标签，并重置攻击计时器
        if (stats.atk_timer_ >= stats.atk_interval_) {
            registry_.emplace_or_replace<game::defs::AttackReadyTag>(entity);
            stats.atk_timer_ = 0.0f;
        }
    }
}

void TimerSystem::updateSkillCooldownTimer(float delta_time) {
    // 筛选条件：有SkillComponent组件，但没有SkillReadyTag标签（即技能正在冷却），排除被动技能
    auto view_skill = registry_.view<game::component::SkillComponent>(
        entt::exclude<game::defs::SkillReadyTag, game::defs::PassiveSkillTag>
    );
    for (auto entity : view_skill) {
        auto& skill = view_skill.get<game::component::SkillComponent>(entity);
        skill.cooldown_timer_ += delta_time;
        // 如果技能冷却计时器大于等于技能冷却时间，代表冷却结束。添加“可施放”标签，并重置技能冷却计时器
        if (skill.cooldown_timer_ >= skill.cooldown_) {
            registry_.emplace_or_replace<game::defs::SkillReadyTag>(entity);
            skill.cooldown_timer_ = 0.0f;
            // 发送技能准备就绪事件
            dispatcher_.enqueue(game::defs::SkillReadyEvent{entity});
        }
    }
}

void TimerSystem::updateSkillDurationTimer(float delta_time) {
    // 筛选条件：有SkillComponent组件，且有SkillActiveTag标签（即技能正在激活中），排除被动技能
    auto view_skill = registry_.view<game::component::SkillComponent,
        game::defs::SkillActiveTag>(entt::exclude<game::defs::PassiveSkillTag>);
    for (auto entity : view_skill) {
        auto& skill = view_skill.get<game::component::SkillComponent>(entity);
        skill.duration_timer_ += delta_time;
        // 如果技能持续计时器大于等于技能持续时间，代表持续结束。移除“技能激活”标签，并重置技能持续计时器
        if (skill.duration_timer_ >= skill.duration_) {
            registry_.remove<game::defs::SkillActiveTag>(entity);
            skill.duration_timer_ = 0.0f;
            // 发送技能持续结束事件
            dispatcher_.enqueue(game::defs::SkillDurationEndEvent{entity});
        }
    }
}

}   // namespace game::system