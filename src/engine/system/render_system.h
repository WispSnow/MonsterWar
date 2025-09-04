#pragma once
#include <entt/entt.hpp>

namespace engine::render {
    class Renderer;
    class Camera;
}

namespace engine::system {

/**
 * @brief 渲染系统
 * 
 * 负责遍历所有带有 TransformComponent 和 SpriteComponent 的实体，
 * 并使用 Renderer 将它们绘制到屏幕上。
 */
class RenderSystem {
public:
    /**
     * @brief 更新渲染系统
     * 
     * @param registry entt::registry 的引用
     * @param renderer Renderer 的引用
     * @param camera Camera 的引用
     */
    void update(entt::registry& registry, render::Renderer& renderer, const render::Camera& camera);
};

} // namespace engine::system 