#include "effect_system.h"
#include "../defs/tags.h"
#include "../defs/events.h"
#include "../factory/entity_factory.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

namespace game::system {

EffectSystem::~EffectSystem() {
    dispatcher_.disconnect(this);
}

EffectSystem::EffectSystem(entt::registry& registry, entt::dispatcher& dispatcher, game::factory::EntityFactory& entity_factory)
    : registry_(registry), dispatcher_(dispatcher), entity_factory_(entity_factory) {
    dispatcher_.sink<game::defs::EnemyDeadEffectEvent>().connect<&EffectSystem::onEnemyDeadEffectEvent>(this);
}

void EffectSystem::onEnemyDeadEffectEvent(const game::defs::EnemyDeadEffectEvent& event) {
    entity_factory_.createEnemyDeadEffect(event.class_id_, event.position_, event.is_flipped_);
}

} // namespace game::system