#pragma once

namespace game::component {

/// @brief COST恢复组件，其中COST会根据时间自动恢复
struct CostRegenComponent {
    float rate_{};          ///< @brief COST恢复速率/秒
};

}   // namespace game::component