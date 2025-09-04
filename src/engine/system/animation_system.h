#pragma once

#include <entt/entity/fwd.hpp>

namespace engine::system {

/**
 * @brief 动画系统
 * 
 * 负责更新实体的动画组件，并同步到精灵组件。
 */
class AnimationSystem {
public:
    /**
     * @brief 更新所有拥有动画和精灵组件的实体
     * @param registry entt注册表
     * @param dt 增量时间
     */
    void update(entt::registry& registry, float dt);
};

} // namespace engine::system