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

/// @brief (通用)特效事件
struct EffectEvent {
    entt::id_type name_id_{entt::null};    ///< @brief 特效ID
    glm::vec2 position_{};                 ///< @brief 位置
    bool is_flipped_{false};               ///< @brief 是否翻转
};

/// @brief (创建)准备单位事件
struct PrepUnitEvent {
    entt::id_type name_id_{entt::null};     ///< @brief 单位名称ID
    entt::id_type class_id_{entt::null};    ///< @brief 职业ID
    int cost_{0};                           ///< @brief 费用
};

/// @brief 移除角色肖像事件
struct RemoveUIPortraitEvent {
    entt::id_type name_id_{entt::null};     ///< @brief 角色名称ID
};

/// @brief 移除玩家单位事件
struct RemovePlayerUnitEvent {
    entt::entity entity_{entt::null};       ///< @brief 单位实体
};

/// @brief 技能准备就绪事件
struct SkillReadyEvent {
    entt::entity entity_{entt::null};       ///< @brief 单位实体
};

/// @brief 技能激活事件
struct SkillActiveEvent {
    entt::entity entity_{entt::null};       ///< @brief 单位实体
};

/// @brief 技能持续结束事件
struct SkillDurationEndEvent {
    entt::entity entity_{entt::null};       ///< @brief 单位实体
};

/// @brief UI肖像悬鼠标浮进入事件
struct UIPortraitHoverEnterEvent {
    entt::id_type name_id_{entt::null};     ///< @brief 角色名称ID
};

/// @brief UI肖像悬鼠标浮离开事件
struct UIPortraitHoverLeaveEvent {};

/// @brief 升级单位事件
struct UpgradeUnitEvent {
    entt::entity entity_{entt::null};       ///< @brief 单位实体
    int cost_{0};                           ///< @brief 消耗费用
};

/// @brief 撤退单位事件
struct RetreatEvent {
    entt::entity entity_{entt::null};       ///< @brief 单位实体
    int cost_{0};                           ///< @brief 返还费用
};

// --- 场景相关事件 ---
struct RestartEvent {};
struct BackToTitleEvent {};
struct SaveEvent {};
struct LevelClearEvent {};

}   // namespace game::defs