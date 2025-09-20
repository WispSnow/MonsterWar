#pragma once

namespace game::data {

/**
 * @brief 关卡内游戏资源及统计数据
 * 
 * 包含可用cost、cost生成速率、基地血量、敌人数量、敌人到达数量、敌人击杀数量等。
 */
struct GameStats {
    float cost_{10.0f};                 ///< @brief 可用cost
    float cost_gen_per_second_{1.0f};   ///< @brief cost生成速率
    int home_hp_{5};                    ///< @brief 基地血量
    int enemy_count_{0};                ///< @brief 敌人(总)数量
    int enemy_arrived_count_{0};        ///< @brief 敌人到达数量
    int enemy_killed_count_{0};         ///< @brief 敌人击杀数量
};

}   // namespace game::data