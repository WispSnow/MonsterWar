#pragma once
#include <entt/entity/entity.hpp>

namespace game::defs {

struct EnemyArriveHomeEvent {};         ///< @brief 敌人到达基地的事件

/// @brief 攻击（命中）事件
struct AttackEvent {
    entt::entity attacker_{entt::null}; ///< @brief 攻击者
    entt::entity target_{entt::null};   ///< @brief 目标
    float damage_{};                    ///< @brief 原始伤害
};

/// @brief 治疗（命中）事件
struct HealEvent {
    entt::entity healer_{entt::null};   ///< @brief 治疗者
    entt::entity target_{entt::null};   ///< @brief 目标
    float amount_{};                    ///< @brief 治疗量
};

}   // namespace game::defs