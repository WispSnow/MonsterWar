#include "remove_dead_system.h"
#include "../defs/tags.h"
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace game::system {

void RemoveDeadSystem::update(entt::registry& registry) {
    // 标签本质上是空的组件，因此操作逻辑和组件一样
    auto view = registry.view<game::defs::DeadTag>();
    for (auto entity : view) {
        registry.destroy(entity);
        spdlog::info("RemoveDeadSystem::update 清理了死亡实体: {}", entt::to_integral(entity));
    }
}

} // namespace game::system