#include "animation_state_system.h"
#include "game/component/enemy_component.h"
#include "game/component/player_component.h"
#include "game/component/blocked_by_component.h"
#include "game/component/skill_component.h"
#include "game/defs/tags.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::system {

AnimationStateSystem::AnimationStateSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<engine::utils::AnimationFinishedEvent>().connect<&AnimationStateSystem::onAnimationFinishedEvent>(this);
}

AnimationStateSystem::~AnimationStateSystem() {
    dispatcher_.disconnect(this);
}

void AnimationStateSystem::onAnimationFinishedEvent(const engine::utils::AnimationFinishedEvent& event) {

    if (!registry_.valid(event.entity_)) return;
    // 敌人动画结束逻辑
    if (registry_.all_of<game::component::EnemyComponent>(event.entity_)) {
        // 如果敌人被阻挡，则返回idle动画
        if (auto blocked_by = registry_.try_get<game::component::BlockedByComponent>(event.entity_); blocked_by) {
            dispatcher_.enqueue(engine::utils::PlayAnimationEvent{event.entity_, "idle"_hs, true});
            spdlog::info("敌人行动动画结束, 返回idle动画, ID: {}", entt::to_integral(event.entity_));
        // 如果没有被阻挡，则返回walk动画
        } else {
            dispatcher_.enqueue(engine::utils::PlayAnimationEvent{event.entity_, "walk"_hs, true});
            spdlog::info("敌人行动动画结束, 没有BlockedBy组件, 返回walk动画, ID: {}", entt::to_integral(event.entity_));
        }
        // 移除动作锁定（硬直）标签
        registry_.remove<game::defs::ActionLockTag>(event.entity_);
        return;
    }

    // 玩家动画结束
    if (registry_.all_of<game::component::PlayerComponent>(event.entity_)) {
        // 如果技能是盾御，且技能正在激活中，则返回guard动画
        const auto& skill = registry_.get<game::component::SkillComponent>(event.entity_);
        if (skill.skill_id_ == "shield"_hs && registry_.any_of<game::defs::SkillActiveTag>(event.entity_)) {
            dispatcher_.enqueue(engine::utils::PlayAnimationEvent{event.entity_, "guard"_hs, true});
            spdlog::info("玩家动画结束, 返回shield动画, ID: {}", entt::to_integral(event.entity_));
        }
        else {  // 其它情况则返回idle动画
            dispatcher_.enqueue(engine::utils::PlayAnimationEvent{event.entity_, "idle"_hs, true});
            spdlog::info("玩家动画结束, 返回idle动画, ID: {}", entt::to_integral(event.entity_));
        }
        // 移除动作锁定（硬直）标签
        registry_.remove<game::defs::ActionLockTag>(event.entity_);
        return;
    }

    // 如果是一次性动画实体（例如死亡特效），则标记死亡待移除
    if (registry_.all_of<game::defs::OneShotRemoveTag>(event.entity_)) {
        registry_.emplace_or_replace<game::defs::DeadTag>(event.entity_);
        return;
    }
}

} // namespace game::system


