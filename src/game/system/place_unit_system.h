#pragma once
#include "game/defs/events.h"
#include "game/defs/constants.h"
#include <entt/entity/entity.hpp>
#include <entt/signal/fwd.hpp>

namespace engine::core {
    class Context;
}

namespace game::factory {
    class EntityFactory;
}

namespace game::system {

/**
 * @brief 放置单位系统
 * @note 处理玩家角色的“准备单位”与“放置地图单位”操作。
 *   - 注意不要混淆“准备单位”（跟随鼠标的待放置单位）与“地图单位”（实际放置在地图上的战斗单位）
 */
class PlaceUnitSystem {
    entt::registry& registry_;
    game::factory::EntityFactory& entity_factory_;
    engine::core::Context& context_;

    /// @brief 目标放置位置实体(即地图上的近战/远程图标)，玩家角色只能放置在这些位置上
    entt::entity target_place_entity_{entt::null};

public:
    PlaceUnitSystem(entt::registry& registry, 
                    game::factory::EntityFactory& entity_factory,
                    engine::core::Context& context);
    ~PlaceUnitSystem();

    void update(float delta_time);

private:
    /**
     * @brief 检查目标位置是否是有效地点实体
     * @note 如果有效，则target_place_entity_指向该实体，否则为null
     *
     * @param position 要检查的位置
     * @param player_type 单位类型
     */
    void checkTargetPlace(const glm::vec2& position, game::defs::PlayerType player_type);

    // 事件回调函数
    void onPrepUnitEvent(const game::defs::PrepUnitEvent& event);           ///< @brief 准备单位事件
    void onRemoveUnitEvent(const game::defs::RemovePlayerUnitEvent& event); ///< @brief 移除(地图上)玩家单位事件

    // 输入控制回调函数
    bool onPlaceUnit();         ///< @brief 放置单位（鼠标左键点击）
    bool onCancelPrepUnit();    ///< @brief 取消放置准备（鼠标右键点击）
};

}