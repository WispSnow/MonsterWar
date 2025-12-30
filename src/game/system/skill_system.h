#pragma once
#include "game/defs/events.h"
#include <entt/signal/fwd.hpp>
#include <entt/entity/fwd.hpp>

namespace game::factory {
    class EntityFactory;
}

namespace game::system {

/**
 * @brief 技能系统
 * @note 用于管理技能的施放与显示、Buff增删等操作
 */
class SkillSystem {
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;
    game::factory::EntityFactory& entity_factory_;

public:
    SkillSystem(entt::registry& registry, entt::dispatcher& dispatcher, game::factory::EntityFactory& entity_factory);
    ~SkillSystem();

private:
    // 事件回调函数
    void onSkillReadyEvent(const game::defs::SkillReadyEvent& event);
    void onSkillActiveEvent(const game::defs::SkillActiveEvent& event);
    void onSkillDurationEndEvent(const game::defs::SkillDurationEndEvent& event);
    void onRemoveUnitEvent(const game::defs::RemovePlayerUnitEvent& event);

    // Buff增删函数
    void addBuff(entt::entity entity, entt::id_type skill_id);
    void removeBuff(entt::entity entity, entt::id_type skill_id);
};

}   // namespace game::system