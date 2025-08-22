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

// 新增一个组件来存储实体的“标签”
// 这里用 entt::hashed_string 作为标签的类型
struct tag {
    entt::hashed_string value;
};


int main() {
    // 使用 using namespace 来简化哈希字符串字面量的使用
    using namespace entt::literals;

    entt::registry registry;

    // 创建实体并添加基础组件
    entt::entity player = registry.create();
    registry.emplace<position>(player, 10.f, 20.f);
    registry.emplace<velocity>(player, 1.f, 0.5f);
    
    entt::entity enemy = registry.create();
    registry.emplace<position>(enemy, 100.f, 50.f);
    registry.emplace<velocity>(enemy, -0.5f, -1.f);
    
    // 1. 使用哈希字符串作为组件数据
    // "player"_hs 和 "enemy"_hs 在编译时就会被转换成一个整数。
    // 这意味着在运行时，我们比较的是两个整数，速度极快。
    registry.emplace<tag>(player, "player"_hs);
    // registry.emplace<tag>(enemy, "enemy"_hs);

    // 运行时转换
    std::string config_name = "enemy";
    auto enemy_hs = entt::hashed_string{config_name.c_str()};
    registry.emplace<tag>(enemy, enemy_hs);

    spdlog::info("=== 使用哈希字符串标签 ===");

    // 2. 通过视图遍历并识别实体
    auto view = registry.view<const tag, const position>();
    
    view.each([](const auto& entity_tag, const auto& pos) {
        // 我们可以直接比较哈希值
        if (entity_tag.value == "player"_hs) {
            spdlog::info("找到玩家，位置: ({}, {})", pos.x, pos.y);
        }

        if (entity_tag.value == "enemy"_hs) {
            spdlog::info("找到敌人，位置: ({}, {})", pos.x, pos.y);
        }
    });

    // 3. 哈希字符串的哈希值与原始值
    auto player_tag = registry.get<tag>(player);
    spdlog::info("玩家标签的哈希值: {}", player_tag.value.value());
    spdlog::info("玩家标签的原始文本: {}", player_tag.value.data());

    return 0;
}