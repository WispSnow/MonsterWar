#include <spdlog/spdlog.h>
#include <entt/entt.hpp>

// 组件是一些简单的结构体或类，它们只包含数据。
// 定义两个组件：位置和速度。
struct position {
    float x;
    float y;
};

struct velocity {
    float dx;
    float dy;
};

int main() {
    // 1. 创建一个 Registry
    // Registry 是所有实体和组件的容器，可以看作是你的“游戏世界”。
    entt::registry registry;

    // 2. 创建实体
    // 实体本身只是一个唯一的标识符 (ID)。
    entt::entity player = registry.create();
    entt::entity enemy = registry.create();

    // 3. 向实体添加组件
    // 使用 emplace<ComponentType>(entity, args...) 来添加并初始化一个组件。
    registry.emplace<position>(player, 10.f, 20.f);
    registry.emplace<velocity>(player, 1.f, 0.f);

    registry.emplace<position>(enemy, 100.f, 50.f);     // 敌人暂时没有速度组件

    // 4. 修改组件
    // 获取组件的引用后，可以直接修改它。
    auto& player_pos = registry.get<position>(player);
    spdlog::info("玩家位置: ({}, {})", player_pos.x, player_pos.y);
    player_pos.x += 5.f;
    spdlog::info("玩家位置: ({}, {})", player_pos.x, player_pos.y);

    // 5. 移除组件
    // 使用 remove<ComponentType>(entity) 来移除一个组件。
    registry.remove<velocity>(player);

    // 6. 销毁实体
    // 销毁实体会自动移除它拥有的所有组件。
    registry.destroy(enemy);
    
    // 检查一个实体是否还存在
    if (registry.valid(player)) {
        spdlog::info("玩家实体仍然有效。");
    }
    if (!registry.valid(enemy)) {
        spdlog::info("敌人实体已失效。");
    }

    return 0;
}