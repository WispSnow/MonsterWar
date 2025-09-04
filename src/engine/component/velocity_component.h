#pragma once
#include <glm/vec2.hpp>

namespace engine::component {

/**
 * @brief 速度组件。
 */
struct VelocityComponent {
    glm::vec2 velocity_{};      ///< @brief 速度
};

}