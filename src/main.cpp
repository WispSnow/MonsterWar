#include <spdlog/spdlog.h>
#include <entt/entt.hpp>

struct position {
    float x;
    float y;
};

struct velocity {
    float dx;
    float dy;
};

// 增加一个新组件，用于标记静态物体，帮助我们演示视图的过滤能力。
struct immovable_tag {};

int main() {
    entt::registry registry;

    entt::entity player = registry.create();
    registry.emplace<position>(player, 10.f, 20.f);
    registry.emplace<velocity>(player, 1.f, 0.5f);

    entt::entity enemy = registry.create();
    registry.emplace<position>(enemy, 100.f, 50.f);
    registry.emplace<velocity>(enemy, -0.5f, -1.f);
    
    entt::entity obstacle = registry.create();
    registry.emplace<position>(obstacle, 200.f, 200.f);
    registry.emplace<immovable_tag>(obstacle); // 障碍物有位置，但没有速度，并且不可移动

    spdlog::info("=== 游戏update更新前 ===");
    auto initial_player_pos = registry.get<position>(player);
    spdlog::info("玩家初始位置: ({}, {})", initial_player_pos.x, initial_player_pos.y);
    auto initial_enemy_pos = registry.get<position>(enemy);
    spdlog::info("敌人初始位置: ({}, {})", initial_enemy_pos.x, initial_enemy_pos.y);


    // === 游戏更新 （通常在System中进行）===
    // 1. 创建一个视图
    // 这个视图会包含所有 *同时* 拥有 position 和 velocity 组件的实体。
    // 障碍物 (obstacle) 不会被包含在内，因为它没有 velocity 组件。
    auto view = registry.view<position, velocity>();
    
    spdlog::info("--- 正在更新 {} 个可移动实体 ---", view.size_hint());

    // 2. 遍历视图并更新组件(这是最简单的写法，其它写法可参见 https://github.com/skypjack/entt/wiki/Entity-Component-System#views)
    for (auto entity : view) {
        auto& pos = view.get<position>(entity);
        const auto& vel = view.get<velocity>(entity);
        pos.x += vel.dx;
        pos.y += vel.dy;
    }

    spdlog::info("=== 游戏 update 结束后 ===");
    auto final_player_pos = registry.get<position>(player);
    spdlog::info("玩家更新后位置: ({}, {})", final_player_pos.x, final_player_pos.y);
    auto final_enemy_pos = registry.get<position>(enemy);
    spdlog::info("敌人更新后位置: ({}, {})", final_enemy_pos.x, final_enemy_pos.y);

    return 0;
}