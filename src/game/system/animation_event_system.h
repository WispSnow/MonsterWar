#pragma once
#include "../../engine/utils/events.h"
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace game::system {

/**
 * @brief 动画事件系统，用于处理各种动画事件
 */
class AnimationEventSystem {
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;

public:
    AnimationEventSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~AnimationEventSystem();

private:
    // 事件回调函数
    void onAnimationEvent(const engine::utils::AnimationEvent& event);

    // 拆分不同的事件类型
    void handleHitEvent(const engine::utils::AnimationEvent& event);    ///< @brief 命中事件
    void handleEmitEvent(const engine::utils::AnimationEvent& event);   ///< @brief 发射事件
};

} // namespace game::system