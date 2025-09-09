#pragma once
#include <entt/entity/fwd.hpp>

namespace engine::system {

/**
 * @brief y-sort排序系统
 */
class YSortSystem {
public:
    void update(entt::registry& registry);
};

} // namespace engine::system
