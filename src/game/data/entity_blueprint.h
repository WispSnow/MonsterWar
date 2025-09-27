#pragma once
#include "../defs/constants.h"
#include "../../engine/utils/math.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>

/* 蓝图结构体，为实体工厂提供数据 */
namespace game::data {

/// @brief 属性蓝图, 用于创建属性组件
struct StatsBlueprint {
    float hp_{0};
    float atk_{0};
    float def_{0};
    float range_{0};
    float atk_interval_{0};
};

/// @brief 精灵蓝图, 用于创建精灵组件
struct SpriteBlueprint {
    entt::id_type id_{entt::null};
    std::string path_;
    engine::utils::Rect src_rect_{};
    glm::vec2 size_{0.0f};
    glm::vec2 offset_{0.0f};
    bool face_right_{true};     ///< @brief 角色图片默认朝右，如果朝左就设置为false
};

/// @brief 单一动画的蓝图，多个蓝图构成的关联容器即可用于创建动画组件
struct AnimationBlueprint {
    float ms_per_frame_{0.0f};
    int row_{0};
    std::vector<int> frames_;   ///< @brief 动画帧索引数组
    std::unordered_map<int, entt::id_type> events_;   ///< @brief 动画事件，键为帧索引，值为事件ID
};

/// @brief 声音蓝图, 用于创建声音组件
struct SoundBlueprint {
    std::unordered_map<entt::id_type, entt::id_type> sounds_;
};

/// @brief 玩家蓝图, 用于创建玩家组件、放置类型、阻挡数量等
struct PlayerBlueprint {
    game::defs::PlayerType type_{game::defs::PlayerType::UNKNOWN};
    entt::id_type skill_id_{entt::null};
    bool healer_{false};
    int block_{0};
    int cost_{0};
};

/// @brief 敌人蓝图, 用于创建敌人组件（EnemyComponent）
struct EnemyBlueprint {
    bool ranged_{false};
    float speed_{};
};

/// @brief 显示信息蓝图, 可用于查找对应职业的名称和描述
struct DisplayInfoBlueprint {
    std::string name_;
    std::string description_;
};

/// @brief 玩家职业蓝图, 包含所有必要的子蓝图，用于创建玩家实体中的所有组件
struct PlayerClassBlueprint {
    entt::id_type class_id_{entt::null};
    entt::id_type projectile_id_{entt::null};
    std::string class_name_;
    StatsBlueprint stats_{};
    PlayerBlueprint player_{};
    SoundBlueprint sounds_{};
    SpriteBlueprint sprite_{};
    DisplayInfoBlueprint display_info_{};
    std::unordered_map<entt::id_type, AnimationBlueprint> animations_;
};

/// @brief 敌人类型蓝图, 包含所有必要的子蓝图，用于创建敌人实体中的所有组件
struct EnemyClassBlueprint {
    entt::id_type class_id_{entt::null};
    entt::id_type projectile_id_{entt::null};
    std::string class_name_;
    StatsBlueprint stats_{};
    EnemyBlueprint enemy_{};
    SoundBlueprint sounds_{};
    SpriteBlueprint sprite_{};
    DisplayInfoBlueprint display_info_{};
    std::unordered_map<entt::id_type, AnimationBlueprint> animations_;
};

/// @brief 投射物蓝图, 用于创建投射物组件
struct ProjectileBlueprint {
    entt::id_type id_{entt::null};
    std::string name_;
    float arc_height_{};
    float total_flight_time_{};
    SpriteBlueprint sprite_{};
    SoundBlueprint sounds_{};
};

/// @brief 特效蓝图, 生成特效实体时使用
struct EffectBlueprint {
    entt::id_type id_{entt::null};
    std::string name_;
    SpriteBlueprint sprite_{};
    AnimationBlueprint animation_{};
};

}   // namespace game::data