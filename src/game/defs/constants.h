#pragma once
#include "../../engine/utils/math.h"
#include <glm/vec2.hpp>

namespace game::defs {

constexpr float BLOCK_RADIUS = 40.0f;   ///< @brief 阻挡半径
constexpr float UNIT_RADIUS = 20.0f;    ///< @brief 角色自身半径（相当于碰撞盒，用于计算攻击范围）
constexpr float PLACE_RADIUS = 40.0f;   ///< @brief 放置区域半径（相当于碰撞盒，用于检测鼠标是否处在可放置位置）

constexpr engine::utils::FColor RANGE_COLOR = {         ///< @brief 攻击范围显示的颜色（RGBA）
    0.0f, 1.0f, 0.0f, 0.3f      // 透明绿色
};

constexpr glm::vec2 HEALTH_BAR_SIZE = {48.0f, 8.0f};    ///< @brief 血量条大小
constexpr float HEALTH_BAR_OFFSET_Y = 8.0f;             ///< @brief 血量条竖直方向偏移量（水平方向默认正中间）

/// @brief 玩家类型枚举
enum class PlayerType {
    UNKNOWN,
    MELEE,      ///< @brief 近战型，只能放在近战区域
    RANGED,     ///< @brief 远程型，只能放在远程区域
    MIXED       ///< @brief 混合型，可以放在任意区域（暂不实现，未来可拓展）
};

}   // namespace game::defs