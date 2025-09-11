#pragma once

namespace game::component {

/**
 * @brief 属性组件
 * 用于存储角色的属性，包括生命值、攻击力、防御力、
 * 攻击范围、攻击间隔、攻击计时器、等级和稀有度。
 */
struct StatsComponent {
    float hp_{};
    float max_hp_{};
    float atk_{};
    float def_{};
    float range_{};             // 攻击范围（射程）
    float atk_interval_{};      // 攻击间隔（决定攻速）
    float atk_timer_{};         // 攻击计时器
    int level_{1};
    int rarity_{1};             // 稀有度，从1开始（例如1:普通，2:稀有，3:史诗，4:传说，5:神话...）
};

}   // namespace game::component