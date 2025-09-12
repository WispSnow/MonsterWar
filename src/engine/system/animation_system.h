#pragma once
#include "../utils/events.h"
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace engine::system {

/**
 * @brief 动画系统
 * 
 * 负责更新实体的动画组件，并同步到精灵组件。
 */
class AnimationSystem {
    // 将依赖保存为成员变量，方便回调函数使用
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    
public:
    AnimationSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~AnimationSystem();

    void update(float dt);  ///< @brief 现在更新函数只需要传入dt，注册表和dispatcher在构造函数中传入

private:
    void onPlayAnimationEvent(const engine::utils::PlayAnimationEvent& event);  ///< @brief 播放动画事件处理函数
};

} // namespace engine::system