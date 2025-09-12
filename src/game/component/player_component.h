#pragma once

namespace game::component {

/// @brief 玩家组件，存储出击消耗
struct PlayerComponent {
    int cost_{};
};

}   // namespace game::component