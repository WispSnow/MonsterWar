#pragma once
#include <glm/vec2.hpp>
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

/// @brief 发射投射物事件
struct EmitProjectileEvent {
    entt::id_type id_{entt::null};          ///< @brief 投射物ID
    entt::entity target_{entt::null};       ///< @brief 目标实体
    glm::vec2 start_position_{};            ///< @brief 起始位置
    glm::vec2 target_position_{};           ///< @brief 目标位置
    float damage_{};                        ///< @brief 伤害
};

/// @brief 敌人死亡特效事件
struct EnemyDeadEffectEvent {
    entt::id_type class_id_{entt::null};   ///< @brief 敌人ID
    glm::vec2 position_{};                 ///< @brief 位置
    bool is_flipped_{false};               ///< @brief 是否翻转
};

}   // namespace game::defs