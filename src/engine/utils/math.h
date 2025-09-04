#pragma once
#include <glm/vec2.hpp>

namespace engine::utils {

/**
 * @brief 自定义矩形结构体，包含位置和大小。
 */
struct Rect
{
    glm::vec2 position{};
    glm::vec2 size{};
    
    Rect() = default;
    Rect(glm::vec2 position, glm::vec2 size) : position(position), size(size) {}
    Rect(float x, float y, float width, float height) : position(x, y), size(width, height) {}
};

/**
 * @brief 自定义颜色结构体。
 */
struct FColor
{
    float r{};
    float g{};
    float b{};
    float a{};
};

} // namespace engine::utils