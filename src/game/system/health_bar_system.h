#pragma once
#include <entt/entity/fwd.hpp>

namespace engine::render {
    class Renderer;
    class Camera;
}

namespace game::system {

/**
 * @brief 地图血量条系统(渲染)，用于显示角色的血量条
 */
class HealthBarSystem {
public:
    void update(entt::registry& registry, engine::render::Renderer& renderer, engine::render::Camera& camera);
};

} // namespace game::system