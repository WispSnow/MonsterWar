#include "render_system.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "../component/transform_component.h"
#include "../component/sprite_component.h"

namespace engine::system {

void RenderSystem::update(entt::registry& registry, render::Renderer& renderer, const render::Camera& camera) {
    auto view = registry.view<component::TransformComponent, component::SpriteComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<component::TransformComponent>(entity);
        const auto& sprite = view.get<component::SpriteComponent>(entity);
        auto position = transform.position_ + sprite.offset_;   // 位置 = 变换组件的位置 + 精灵的偏移
        auto size = sprite.size_ * transform.scale_;            // 大小 = 精灵的大小 * 变换组件的缩放
        renderer.drawSprite(camera, sprite.sprite_, position, size, transform.rotation_);
    }
}

} // namespace engine::system 