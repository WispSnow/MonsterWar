#include "animation_event_system.h"
#include "../component/enemy_component.h"
#include "../component/player_component.h"
#include "../component/blocked_by_component.h"
#include "../component/target_component.h"
#include "../component/stats_component.h"
#include "../defs/tags.h"
#include "../defs/events.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::system {

AnimationEventSystem::AnimationEventSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<engine::utils::AnimationEvent>().connect<&AnimationEventSystem::onAnimationEvent>(this);
}

AnimationEventSystem::~AnimationEventSystem() {
    dispatcher_.disconnect(this);
}

void AnimationEventSystem::onAnimationEvent(const engine::utils::AnimationEvent& event) {
    if (!registry_.valid(event.entity_)) return;
    // 根据不同的事件id，调用不同的处理函数
    if (event.event_name_id_ == "hit"_hs) {
        handleHitEvent(event);
    }
    // TODO: 其他事件类型
}

void AnimationEventSystem::handleHitEvent(const engine::utils::AnimationEvent& event) {
    // 玩家命中事件：治疗或攻击当前目标
    if (registry_.all_of<game::component::PlayerComponent>(event.entity_)) {
        // 命中时有可能目标已经解锁，因此需要检查
        if (auto target_component = registry_.try_get<game::component::TargetComponent>(event.entity_); target_component) {
            const auto& stats_component = registry_.get<game::component::StatsComponent>(event.entity_);
            // 根据玩家职业，执行治疗或攻击（事件）
            if (registry_.all_of<game::defs::HealerTag>(event.entity_)) {
                dispatcher_.enqueue(game::defs::HealEvent{event.entity_, target_component->entity_, stats_component.atk_});
            } else {
                dispatcher_.enqueue(game::defs::AttackEvent{event.entity_, target_component->entity_, stats_component.atk_});
            }
            // 播放“hit”音效
            dispatcher_.enqueue(engine::utils::PlaySoundEvent{event.entity_, "hit"_hs});
        }
        return;
    }

    // 敌人命中事件：对阻挡者造成伤害
    if (registry_.all_of<game::component::EnemyComponent>(event.entity_)) {
        // 命中时有可能目标（阻挡者）已经解锁，因此需要检查
        if (auto blocked_by_component = registry_.try_get<game::component::BlockedByComponent>(event.entity_); blocked_by_component) {
            const auto& stats_component = registry_.get<game::component::StatsComponent>(event.entity_);
            // 执行攻击事件
            dispatcher_.enqueue(game::defs::AttackEvent{event.entity_, blocked_by_component->entity_, stats_component.atk_});
        }
        // NOTE: 只有远程敌人才有Target组件，但远程攻击动画事件id为"emit"，不在这里处理
        // NOTE: 敌人命中事件不播放音效，未来如果需要可以补充
    }
}

} // namespace game::system