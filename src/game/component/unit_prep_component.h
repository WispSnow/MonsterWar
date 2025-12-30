#pragma once
#include "game/defs/constants.h"
#include <entt/entity/entity.hpp>

namespace game::component {

/**
 * @brief 单位准备组件，存储单位准备信息
 * @note 包括名称、类型、范围、费用
 */
struct UnitPrepComponent {
    entt::id_type name_id_{entt::null};
    game::defs::PlayerType type_{game::defs::PlayerType::UNKNOWN};
    float range_{0};
    int cost_{0};
};

}   // namespace game::component