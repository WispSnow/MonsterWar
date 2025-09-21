#pragma once
#include <entt/entity/fwd.hpp>

namespace engine::render {
    class Renderer;
    class Camera;
}

namespace game::system {

/**
 * @brief 渲染范围系统，根据条件渲染远程角色的攻击范围
 */
class RenderRangeSystem {
public:
    void update(entt::registry& registry, engine::render::Renderer& renderer, const engine::render::Camera& camera);

};

}   // namespace game::system