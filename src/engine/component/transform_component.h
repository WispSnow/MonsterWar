#pragma once
#include <glm/vec2.hpp>
#include <utility>

namespace engine::component {

/**
 * @brief 变换组件，包含位置、缩放和旋转。
 */
struct TransformComponent {
    glm::vec2 position_{};          ///< @brief 位置
    glm::vec2 scale_{1.0f};         ///< @brief 缩放
    float rotation_{};              ///< @brief 旋转

    /**
     * @brief 构造函数
     * @param position 位置
     * @param scale 缩放，默认(1.0f,1.0f)
     * @param rotation 旋转，默认0
     */
    explicit TransformComponent(glm::vec2 position, 
                       glm::vec2 scale = glm::vec2(1.0f, 1.0f), 
                       float rotation = 0.0f) : 
                       position_(std::move(position)), 
                       scale_(std::move(scale)), 
                       rotation_(rotation) {}
};

}