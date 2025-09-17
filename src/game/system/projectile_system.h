#pragma once
#include "../defs/events.h"
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace game::factory {
    class EntityFactory;
}

namespace game::system {

/**
 * @brief 投射物系统
 * 1. 相响应投射物创建事件，创建投射物实体
 * 2. 更新投射物的飞行状态，并发送攻击事件和播放音效
 */
class ProjectileSystem {
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    game::factory::EntityFactory& entity_factory_;  ///< @brief 需要传入实体工厂引用，负责创建投射物实体

public:
    ProjectileSystem(entt::registry& registry, entt::dispatcher& dispatcher, game::factory::EntityFactory& entity_factory);
    ~ProjectileSystem();

    void update(float delta_time);

private:
    // 事件回调函数
    void onEmitProjectileEvent(const game::defs::EmitProjectileEvent& event);
};

}   // namespace game::system