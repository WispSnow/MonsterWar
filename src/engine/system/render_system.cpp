#include "render_system.h"
#include "engine/render/renderer.h"
#include "engine/render/camera.h"
#include "engine/component/transform_component.h"
#include "engine/component/sprite_component.h"
#include "engine/component/render_component.h"
#include <spdlog/spdlog.h>

namespace engine::system {

void RenderSystem::update(entt::registry& registry, render::Renderer& renderer, const render::Camera& camera) {
    spdlog::trace("RenderSystem::update");

    // 对 RenderComponent storage 排序，比较规则由 RenderComponent::operator< 定义。
    registry.sort<component::RenderComponent>([](const auto& lhs, const auto& rhs) {
        return lhs < rhs;
    });

    // EnTT 的 view 默认会选择元素最少的 storage 驱动迭代。
    // 显式指定使用 RenderComponent，才能保证遍历顺序与上面的排序一致。
    auto view = registry.view<component::RenderComponent, component::TransformComponent, component::SpriteComponent>();
    view.use<component::RenderComponent>();
    for (auto entity : view) {
        const auto& render = view.get<component::RenderComponent>(entity);
        const auto& transform = view.get<component::TransformComponent>(entity);
        const auto& sprite = view.get<component::SpriteComponent>(entity);
        auto position = transform.position_ + sprite.offset_;   // 位置 = 变换组件的位置 + 精灵的偏移
        auto size = sprite.size_ * transform.scale_;            // 大小 = 精灵的大小 * 变换组件的缩放
        // 绘制时应用Render组件中的颜色调整参数
        renderer.drawSprite(camera, sprite.sprite_, position, size, transform.rotation_, render.color_);
    }
}

} // namespace engine::system 
