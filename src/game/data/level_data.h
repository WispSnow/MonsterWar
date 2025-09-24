#pragma once
#include <vector>
#include <queue>    // 队列：容器只能从一端入队，从另一端出队，确保先进先出
#include <utility>
#include <entt/entity/fwd.hpp>
#include <string>

namespace game::data {

/**
 * @brief 单一波次数据
 * @note 包含下一波次间隔、本波次敌人生成间隔和“本波次敌人类型-数量”对
 */
struct Wave {
    float next_wave_interval_{};    ///< @brief 下一波次间隔（单位：秒）
    float spawn_interval_{};        ///< @brief 本波次敌人生成间隔（单位：秒）
    std::vector<std::pair<entt::id_type, int>> enemy_types_; ///< @brief 敌人类型-数量对
};

/**
 * @brief 多波次数据，即一关中所有的波次
 * @note 包含下一波次倒计时、波次队列
 */
struct Waves {
    float next_wave_count_down_{};  ///< @brief 下一波次倒计时（单位：秒）
    std::queue<Wave> waves_;        ///< @brief 波次队列
};

/**
 * @brief 关卡数据，包含一关中的波次数据及其他必要信息
 * @note 关卡号、敌人等级、敌人稀有度、关卡名称、地图路径、准备时间、总敌人数量
 */
struct LevelData {
    int level_number_{1};           ///< @brief 关卡号
    int enemy_level_{1};            ///< @brief 敌人等级（本关所有敌人统一等级）
    int enemy_rarity_{1};           ///< @brief 敌人稀有度（本关所有敌人统一稀有度）
    std::string name_;              ///< @brief 关卡名称
    std::string map_path_;          ///< @brief 地图路径
    float prep_time_{5.0f};         ///< @brief 开局准备时间（单位：秒）
    int total_enemy_count_{0};      ///< @brief 总敌人数量
    Waves waves_data_;              ///< @brief 波次数据
};

}