#pragma once
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>
#include "../../engine/utils/events.h"

namespace game::system {

/**
 * @brief 动画状态系统，用于处理动画播放事件。
 */
class AnimationStateSystem {
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;

public:
    AnimationStateSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~AnimationStateSystem();

    /* 系统可以没有更新函数，只专门处理事件回调 */
    
private:
    // 事件回调函数
    void onAnimationFinishedEvent(const engine::utils::AnimationFinishedEvent& event);
};

} // namespace game::system


