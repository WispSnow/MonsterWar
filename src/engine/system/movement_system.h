#pragma once
#include <entt/entity/registry.hpp>

namespace engine::system {

/**
 * @brief 移动系统
 * 
 * 负责更新实体的移动组件，并同步到变换组件。
 */
class MovementSystem {
public:
    /**
     * @brief 更新所有拥有移动和变换组件的实体
     * @param registry entt注册表
     * @param delta_time 增量时间
     */
    void update(entt::registry& registry, float delta_time);
};
}