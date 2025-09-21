#pragma once
#include <entt/entity/entity.hpp>

namespace game::component {

/**
 * @brief "位置被占用"组件，存储被放置的单位实体
 * @note 添加到地点实体中，一个地点只能放置一个单位
 */
struct PlaceOccupiedComponent {
    entt::entity entity_{entt::null};
};

}   // namespace game::component