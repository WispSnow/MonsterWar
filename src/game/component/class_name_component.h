#pragma once
#include <entt/entity/entity.hpp>
#include <string>

namespace game::component {

/**
 * @brief 职业名称组件。
 * 用于存储玩家职业（例如战士、法师、弓箭手） 或 
 * 敌人类型（例如史莱姆、狼、哥布林）。的ID和名称
 */
struct ClassNameComponent {
    entt::id_type class_id_{entt::null};
    std::string class_name_;                // 可以是中文，主要用于显示
};

}   // namespace game::component