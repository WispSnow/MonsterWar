#include "combat_resolve_system.h"
#include "../component/stats_component.h"
#include "../component/player_component.h"
#include "../component/enemy_component.h"
#include "../component/blocked_by_component.h"
#include "../component/blocker_component.h"
#include "../defs/tags.h"
#include "../defs/events.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <glm/common.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::system {

CombatResolveSystem::CombatResolveSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<game::defs::AttackEvent>().connect<&CombatResolveSystem::onAttackEvent>(this);
    dispatcher_.sink<game::defs::HealEvent>().connect<&CombatResolveSystem::onHealEvent>(this);
}

CombatResolveSystem::~CombatResolveSystem() {
    dispatcher_.disconnect(this);
}

void CombatResolveSystem::onAttackEvent(const game::defs::AttackEvent& event) {
    // 如果目标无效或标记死亡，直接返回
    if (!registry_.valid(event.target_) || registry_.all_of<game::defs::DeadTag>(event.target_)) return;
    // 根据伤害公式，让目标扣血
    auto& target_stats = registry_.get<game::component::StatsComponent>(event.target_);
    float damage = calculateEffectiveDamage(event.damage_, target_stats.def_);
    target_stats.hp_ -= damage;

    // 如果目标是玩家
    if (registry_.all_of<game::component::PlayerComponent>(event.target_)) {
        spdlog::info("玩家 ID: {} 受到 ID: {} 的伤害, 剩余生命值: {}", 
            entt::to_integral(event.target_), entt::to_integral(event.attacker_), target_stats.hp_);
        // 死亡情况
        if (target_stats.hp_ <= 0) {
            target_stats.hp_ = 0;
            // 用emplace重复添加会报错，用emplace_or_replace更加健壮，可重复添加
            registry_.emplace_or_replace<game::defs::DeadTag>(event.target_);
            spdlog::info("玩家 ID: {} 死亡", entt::to_integral(event.target_));
            // NOTE: 可添加死亡特效, 统计信息等
        // 受伤情况
        } else if (target_stats.hp_ < target_stats.max_hp_) {
            registry_.emplace_or_replace<game::defs::InjuredTag>(event.target_);
        }
        return;
    }

    // 如果目标是敌人
    if (registry_.all_of<game::component::EnemyComponent>(event.target_)) {
        spdlog::info("敌人 ID: {} 受到 ID: {} 的伤害, 剩余生命值: {}", 
            entt::to_integral(event.target_), entt::to_integral(event.attacker_), target_stats.hp_);
        // 死亡情况
        if (target_stats.hp_ <= 0) {
            target_stats.hp_ = 0;
            registry_.emplace_or_replace<game::defs::DeadTag>(event.target_);
            spdlog::info("敌人 ID: {} 死亡", entt::to_integral(event.target_));
            // TODO: 添加死亡特效
            // TODO: 更新统计信息
            // 如果敌人被阻挡，减少阻挡者的阻挡计数
            if (auto blocked_by = registry_.try_get<game::component::BlockedByComponent>(event.target_); blocked_by) {
                auto blocker_entity = blocked_by->entity_;
                if (registry_.valid(blocker_entity)) {
                    auto& blocker = registry_.get<game::component::BlockerComponent>(blocker_entity);
                    blocker.current_count_ = glm::max(0, blocker.current_count_ - 1);
                }
            }
        // 受伤情况
        } else if (target_stats.hp_ < target_stats.max_hp_) {
            registry_.emplace_or_replace<game::defs::InjuredTag>(event.target_);
        }
        return;
    }
}

void CombatResolveSystem::onHealEvent(const game::defs::HealEvent& event) {
    if (!registry_.valid(event.target_)) return;
    if (!registry_.all_of<game::component::PlayerComponent>(event.target_)) return;
    // 根据治疗量，让目标回血
    auto& target_stats = registry_.get<game::component::StatsComponent>(event.target_);
    target_stats.hp_ += event.amount_;
    spdlog::info("治疗者 ID: {}, 治疗目标 ID: {}, 治疗量: {}", 
        entt::to_integral(event.healer_), entt::to_integral(event.target_), event.amount_);
    // 如果治疗后满血，移除受伤标签
    if (target_stats.hp_ >= target_stats.max_hp_) {
        target_stats.hp_ = target_stats.max_hp_;
        registry_.remove<game::defs::InjuredTag>(event.target_);
    }
    // TODO: 添加治疗特效
}

// --- 辅助函数 ---
float CombatResolveSystem::calculateEffectiveDamage(float attacker_atk, float target_def) {
    // 最终伤害 = 攻击力 - 防御力
    float damage = attacker_atk - target_def;
    // 最小伤害为攻击力的10%
    damage = std::max(damage, 0.1f * attacker_atk);
    return damage;
}

} // namespace game::system


