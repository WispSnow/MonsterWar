#include "movement_system.h"
#include "../component/velocity_component.h"
#include "../component/transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::system {

void MovementSystem::update(entt::registry& registry, float delta_time) {
    spdlog::trace("MovementSystem::update");
    // 获取感兴趣的实体 view
    auto view = registry.view<engine::component::VelocityComponent, engine::component::TransformComponent>();

    // 遍历获取的实体，获取组件并执行相关逻辑
    for (auto entity : view) {
        const auto& velocity = view.get<engine::component::VelocityComponent>(entity);
        auto& transform = view.get<engine::component::TransformComponent>(entity);

        transform.position_ += velocity.velocity_ * delta_time; // 更新位置
    }
}

}   // namespace engine::system