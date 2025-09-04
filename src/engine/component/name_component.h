#pragma once

#include <string>
#include <entt/entity/entity.hpp>

namespace engine::component {

/**
 * @brief 名称组件，可用于标记实体名称。
 */
struct NameComponent {
    entt::id_type name_id_{entt::null};   ///< @brief 名称ID
    std::string name_;                    ///< @brief 名称
};

}