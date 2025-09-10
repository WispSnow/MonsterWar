#pragma once
#include <entt/entity/fwd.hpp>

namespace game::system {

/**
 * @brief 清理死亡实体的系统
 */
class RemoveDeadSystem {
public:
    void update(entt::registry& registry);
};

} // namespace game::system
