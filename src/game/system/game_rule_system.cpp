#include "game_rule_system.h"
#include "../data/game_stats.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

namespace game::system {

GameRuleSystem::GameRuleSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {}

GameRuleSystem::~GameRuleSystem() {}

void GameRuleSystem::update(float delta_time) {
    // 更新Cost
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.cost_ += game_stats.cost_gen_per_second_ * delta_time;
    // TODO: 可能的buff效果

}

void GameRuleSystem::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    game_stats.enemy_arrived_count_++;      // 敌人到达数量+1
    game_stats.home_hp_ -= 1;               // 基地血量-1
    if (game_stats.home_hp_ <= 0) {
        spdlog::warn("基地被摧毁");
        // TODO: 切换场景逻辑
    }
}

}   // namespace game::system