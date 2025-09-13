#pragma once
#include <entt/entity/entity.hpp>

namespace game::component {

/**
 * @brief 目标组件，包含目标实体，用于锁定攻击对象。
 */
struct TargetComponent {
    entt::entity entity_{entt::null};
};

}   // namespace game::component