#include "set_target_system.h"
#include "../component/target_component.h"
#include "../component/stats_component.h"
#include "../component/player_component.h"
#include "../component/enemy_component.h"
#include "../defs/tags.h"
#include "../defs/constants.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/utils/math.h"
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace game::system {

void SetTargetSystem::update(entt::registry& registry) {
    updateHasTarget(registry);
    updateNoTargetPlayer(registry);
    updateNoTargetEnemy(registry);
    updateHealer(registry);
}

void SetTargetSystem::updateHasTarget(entt::registry& registry) {
    // 筛选条件：敌我双方所有攻击型角色（排除治疗者，治疗者是另外逻辑）
    auto view_has_target = registry.view<engine::component::TransformComponent, 
        game::component::TargetComponent, 
        game::component::StatsComponent>(entt::exclude<game::defs::HealerTag>);
    // 遍历每一个有目标的角色
    for (auto entity : view_has_target) {
        const auto& target = view_has_target.get<game::component::TargetComponent>(entity);
        const auto& transform = view_has_target.get<engine::component::TransformComponent>(entity);
        const auto& stats = view_has_target.get<game::component::StatsComponent>(entity);
        // 检查目标是否还有效
        if (!registry.valid(target.entity_)) {
            // 如果目标实体无效，则清除目标
            registry.remove<game::component::TargetComponent>(entity);
            spdlog::info("ID: {}, 目标: ID: {}, 无效, 清除目标", 
                         entt::to_integral(entity), 
                         entt::to_integral(target.entity_));
            continue;
        }
        // 检查目标是否还在攻击范围之内（检测半径 = 角色攻击范围 + 目标角色半径）
        const auto& target_transform = registry.get<engine::component::TransformComponent>(target.entity_);
        auto range_radius = stats.range_ + game::defs::UNIT_RADIUS;
        if (engine::utils::distanceSquared(transform.position_, target_transform.position_) > range_radius * range_radius) {
            // 如果在攻击范围外，则清除目标
            registry.remove<game::component::TargetComponent>(entity);
            spdlog::info("ID: {}, 目标: ID: {}, 不在攻击范围之内, 清除目标", entt::to_integral(entity), entt::to_integral(target.entity_));
            continue;
        }
    }
}

void SetTargetSystem::updateNoTargetPlayer(entt::registry& registry) {
    // 筛选条件：没有目标的玩家攻击型角色
    auto view_player_no_target = registry.view<engine::component::TransformComponent, 
        game::component::StatsComponent, 
        game::component::PlayerComponent>(entt::exclude<game::component::TargetComponent, game::defs::HealerTag>);
    // 获取所有敌方角色用于检测
    auto view_enemy = registry.view<engine::component::TransformComponent, game::component::EnemyComponent>();
    // 遍历每一个没有目标的玩家攻击型角色
    for (auto player_entity : view_player_no_target) {
        const auto& player_transform = view_player_no_target.get<engine::component::TransformComponent>(player_entity);
        const auto& player_stats = view_player_no_target.get<game::component::StatsComponent>(player_entity);
        // 检查敌人是否在攻击范围之内
        for (auto enemy_entity : view_enemy) {
            const auto& enemy_transform = view_enemy.get<engine::component::TransformComponent>(enemy_entity);
            auto range_radius = player_stats.range_ + game::defs::UNIT_RADIUS;
            if (engine::utils::distanceSquared(player_transform.position_, enemy_transform.position_) <= range_radius * range_radius) {
                // 如果敌人在攻击范围之内，则设置目标
                registry.emplace<game::component::TargetComponent>(player_entity, enemy_entity);
                spdlog::info("玩家: ID: {}, 设置目标: ID: {}", entt::to_integral(player_entity), entt::to_integral(enemy_entity));
                break;  // 设置一个目标敌人就停止检查
            }
        }
    }
}

void SetTargetSystem::updateNoTargetEnemy(entt::registry& registry) {
    // 筛选条件：没有目标的敌人角色（只考虑远程型，近战敌人的目标就是阻挡者）
    auto view_enemy_no_target = registry.view<game::component::EnemyComponent, 
        engine::component::TransformComponent, 
        game::component::StatsComponent, 
        game::defs::RangedUnitTag>(entt::exclude<game::component::TargetComponent>);
    // 获取所有玩家角色用于检测
    auto view_player = registry.view<engine::component::TransformComponent, game::component::PlayerComponent>();
    // 遍历每一个没有目标的敌人角色
    for (auto enemy_entity : view_enemy_no_target) {
        const auto& enemy_transform = view_enemy_no_target.get<engine::component::TransformComponent>(enemy_entity);
        const auto& enemy_stats = view_enemy_no_target.get<game::component::StatsComponent>(enemy_entity);
        // 检查玩家角色是否在攻击范围之内
        for (auto player_entity : view_player) {
            const auto& player_transform = view_player.get<engine::component::TransformComponent>(player_entity);
            auto range_radius = enemy_stats.range_ + game::defs::UNIT_RADIUS;
            if (engine::utils::distanceSquared(enemy_transform.position_, player_transform.position_) <= range_radius * range_radius) {
                // 如果玩家角色在攻击范围之内，则设置目标
                registry.emplace<game::component::TargetComponent>(enemy_entity, player_entity);
                spdlog::info("敌人: ID: {}, 设置目标: ID: {}", entt::to_integral(enemy_entity), entt::to_integral(player_entity));
                break;  // 设置一个目标玩家角色就停止检查
            }
        }
    }
}

void SetTargetSystem::updateHealer(entt::registry& registry) {
    // --- 检查治疗者(玩家角色)的目标，选择血量百分比最低的受伤玩家角色作为目标 ---
    // 筛选条件：玩家治疗者角色
    auto view_healer = registry.view<game::defs::HealerTag, 
        game::component::PlayerComponent,
        engine::component::TransformComponent,
        game::component::StatsComponent>();
    // 获取所有受伤玩家角色用于检测
    auto view_injured_player = registry.view<game::component::PlayerComponent, 
        game::component::StatsComponent, 
        game::defs::InjuredTag,
        engine::component::TransformComponent>();
    // 遍历每一个治疗者
    for (auto healer_entity : view_healer) {
        auto& healer_stats = registry.get<game::component::StatsComponent>(healer_entity);
        auto& healer_transform = registry.get<engine::component::TransformComponent>(healer_entity);
        // ---获取血量百分比最低的玩家角色---
        float lowest_hp_percent = 1.0f;             // 保存最低血量百分比（初始为100%）
        entt::entity lowest_hp_player = entt::null; // 保存最低血量百分比的玩家角色（初始为空）
        // 遍历每一个受伤玩家角色
        for (auto player_entity : view_injured_player) {
            // 如果处于治疗者攻击范围内
            const auto& player_transform = view_injured_player.get<engine::component::TransformComponent>(player_entity);
            auto range_radius = healer_stats.range_ + game::defs::UNIT_RADIUS;
            if (engine::utils::distanceSquared(healer_transform.position_, player_transform.position_) < range_radius * range_radius) {
                // 计算血量百分比并更新最低百分比和目标角色
                const auto& player_stats = view_injured_player.get<game::component::StatsComponent>(player_entity);
                auto hp_percent = static_cast<float>(player_stats.hp_) / static_cast<float>(player_stats.max_hp_);
                if (hp_percent < lowest_hp_percent) {
                    lowest_hp_percent = hp_percent;
                    lowest_hp_player = player_entity;
                }
            }
        }
        // 如果找到了最低血量百分比的玩家角色，则设置目标
        if (lowest_hp_player != entt::null) {
            // 设置（更新）目标
            registry.emplace_or_replace<game::component::TargetComponent>(healer_entity, lowest_hp_player);
        }
        // 否则移除目标(即使没有组件，也可以安全调用remove)
        else {
            registry.remove<game::component::TargetComponent>(healer_entity);
        }
    }
}

}   // namespace game::system