#pragma once

namespace game::defs {

constexpr float BLOCK_RADIUS = 40.0f;   ///< @brief 阻挡半径

/// @brief 玩家类型枚举
enum class PlayerType {
    UNKNOWN,
    MELEE,      ///< @brief 近战型，只能放在近战区域
    RANGED,     ///< @brief 远程型，只能放在远程区域
    MIXED       ///< @brief 混合型，可以放在任意区域（暂不实现，未来可拓展）
};

}   // namespace game::defs