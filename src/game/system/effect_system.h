#pragma once
#include "game/defs/events.h"
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace game::factory {
    class EntityFactory;
}

namespace game::system {

/**
 * @brief 特效系统，处理所有特效的创建
 */
class EffectSystem {
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    game::factory::EntityFactory& entity_factory_;

public:
    EffectSystem(entt::registry& registry, entt::dispatcher& dispatcher, game::factory::EntityFactory& entity_factory);
    ~EffectSystem();

private:
    // 事件回调函数
    void onEnemyDeadEffectEvent(const game::defs::EnemyDeadEffectEvent& event); ///< @brief 敌人死亡特效事件
    void onEffectEvent(const game::defs::EffectEvent& event);                   ///< @brief (通用)特效事件
    // TODO: 未来添加其他特效事件回调函数
    
};
} // namespace game::system