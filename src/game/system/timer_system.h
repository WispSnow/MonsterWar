#pragma once
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace game::system {

/**
 * @brief 计时器系统，用于更新所有包含计时器的组件，
 * 并在满足条件时添加必要的标签，（如攻击冷却完成后，添加“可攻击”标签）。
 */
class TimerSystem {
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;

public:
    TimerSystem(entt::registry& registry, entt::dispatcher& dispatcher);

    void update(float delta_time);

private:
    // 拆分逻辑的函数，在update中调用
    void updateAttackTimer(float delta_time); ///< @brief 处理攻击计时器
    void updateSkillCooldownTimer(float delta_time); ///< @brief 处理技能冷却计时器
    void updateSkillDurationTimer(float delta_time); ///< @brief 处理技能持续计时器
    // TODO: 处理其他计时器
};

}