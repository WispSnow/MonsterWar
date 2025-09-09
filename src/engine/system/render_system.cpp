#include "render_system.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "../component/transform_component.h"
#include "../component/sprite_component.h"
#include "../component/render_component.h"
#include <spdlog/spdlog.h>

namespace engine::system {

void RenderSystem::update(entt::registry& registry, render::Renderer& renderer, const render::Camera& camera) {
    spdlog::trace("RenderSystem::update");

    // 对RenderComponent进行排序 (需要自定义RenderComponent的比较运算符)
    registry.sort<component::RenderComponent>([](const auto& lhs, const auto& rhs) {
        return lhs < rhs;
    });

    // 执行渲染，注意排序组件RenderComponent必须放在最前面
    auto view = registry.view<component::RenderComponent, component::TransformComponent, component::SpriteComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<component::TransformComponent>(entity);
        const auto& sprite = view.get<component::SpriteComponent>(entity);
        auto position = transform.position_ + sprite.offset_;   // 位置 = 变换组件的位置 + 精灵的偏移
        auto size = sprite.size_ * transform.scale_;            // 大小 = 精灵的大小 * 变换组件的缩放
        renderer.drawSprite(camera, sprite.sprite_, position, size, transform.rotation_);
    }
}

} // namespace engine::system 