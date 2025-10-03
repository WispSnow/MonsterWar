#include "game_rule_system.h"
#include "../data/game_stats.h"
#include "../factory/blueprint_manager.h"
#include "../component/cost_regen_component.h"
#include "../component/stats_component.h"
#include "../component/class_name_component.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/utils/math.h"
#include "../../engine/utils/events.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::system {

GameRuleSystem::GameRuleSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<game::defs::EnemyArriveHomeEvent>().connect<&GameRuleSystem::onEnemyArriveHome>(this);
    dispatcher_.sink<game::defs::UpgradeUnitEvent>().connect<&GameRuleSystem::onUpgradeUnitEvent>(this);
    dispatcher_.sink<game::defs::RetreatEvent>().connect<&GameRuleSystem::onRetreatEvent>(this);
    dispatcher_.sink<game::defs::LevelClearDelayedEvent>().connect<&GameRuleSystem::onLevelClearDelayedEvent>(this);
}

GameRuleSystem::~GameRuleSystem() {
    dispatcher_.disconnect(this);
}

void GameRuleSystem::update(float delta_time) {
    // 更新Cost
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.cost_ += game_stats.cost_gen_per_second_ * delta_time;
    // 更新COST恢复
    auto view_cost_regen = registry_.view<game::component::CostRegenComponent>();
    for (auto entity : view_cost_regen) {
        auto& cost_regen = view_cost_regen.get<game::component::CostRegenComponent>(entity);
        game_stats.cost_ += cost_regen.rate_ * delta_time;
    }
    // 如果已经通关，计时器归零后切换场景
    if (is_level_clear_) {
        level_clear_timer_ -= delta_time;
        if (level_clear_timer_ <= 0.0f) {
            dispatcher_.enqueue(game::defs::LevelClearEvent{});
            is_level_clear_ = false;    // 重置关卡通关标志, 避免重复触发
        }
    }
}

void GameRuleSystem::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.enemy_arrived_count_++;      // 敌人到达数量+1
    game_stats.home_hp_ -= 1;               // 基地血量-1
    if (game_stats.home_hp_ <= 0) {
        spdlog::warn("基地被摧毁");
        // 游戏失败
        dispatcher_.enqueue(game::defs::GameEndEvent{false});
    }
    else if ((game_stats.enemy_arrived_count_ + game_stats.enemy_killed_count_) >= game_stats.enemy_count_) {
        // 通关成功，延迟切换场景
        dispatcher_.enqueue(game::defs::LevelClearDelayedEvent{});
    }
}

void GameRuleSystem::onUpgradeUnitEvent(const game::defs::UpgradeUnitEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) return;
    // 扣除COST
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.cost_ -= event.cost_;
    // 获取Stats组件并让其等级 + 1
    auto& stats = registry_.get<game::component::StatsComponent>(event.entity_);
    stats.level_++;
    // 更新属性 (需要从蓝图中获取基础数据，然后根据等级和稀有度修改Stats组件)
    auto& blueprint_mgr = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    const auto& class_name = registry_.get<game::component::ClassNameComponent>(event.entity_);
    const auto& stats_blueprint = blueprint_mgr->getPlayerClassBlueprint(class_name.class_id_).stats_;
    stats.hp_ = engine::utils::statModify(stats_blueprint.hp_, stats.level_, stats.rarity_);
    stats.max_hp_ = engine::utils::statModify(stats_blueprint.hp_, stats.level_, stats.rarity_);
    stats.atk_ = engine::utils::statModify(stats_blueprint.atk_, stats.level_, stats.rarity_);
    stats.def_ = engine::utils::statModify(stats_blueprint.def_, stats.level_, stats.rarity_);
    // 创建特效
    const auto& transform = registry_.get<engine::component::TransformComponent>(event.entity_);
    dispatcher_.enqueue(game::defs::EffectEvent{"level_up"_hs, transform.position_, false});
    // 播放音效
    dispatcher_.enqueue(engine::utils::PlaySoundEvent{event.entity_, "level_up"_hs});
}

void GameRuleSystem::onRetreatEvent(const game::defs::RetreatEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) return;
    // 返还COST
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.cost_ += event.cost_;
    // 发送移除单位事件
    dispatcher_.enqueue(game::defs::RemovePlayerUnitEvent{event.entity_});
}

void GameRuleSystem::onLevelClearDelayedEvent(const game::defs::LevelClearDelayedEvent& event) {
    // 设置关卡通关标志和计时器
    is_level_clear_ = true;
    level_clear_timer_ = event.delay_time_;
}

}   // namespace game::system