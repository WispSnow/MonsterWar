#pragma once
#include <entt/entity/fwd.hpp>

namespace game::system {

/**
 * @brief 设置目标系统，用于设置角色的攻击目标。
 */
class SetTargetSystem {
public:
    void update(entt::registry& registry);

private:
    // 拆分逻辑的函数，在update中调用
    void updateHasTarget(entt::registry& registry);         ///< @brief 处理有目标的角色
    void updateNoTargetPlayer(entt::registry& registry);    ///< @brief 处理没有目标的玩家攻击型角色
    void updateNoTargetEnemy(entt::registry& registry);     ///< @brief 处理没有目标的敌人角色
    void updateHealer(entt::registry& registry);            ///< @brief 处理治疗者
};

}   // namespace game::system