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
// 这里用 entt::hashed_string 作为标签的类型 (不推荐这样用)
// struct tag {
//     entt::hashed_string value;   // 它只保留字符串的“指针”，不保证字符串不会被销毁（类似std::string_view）
// };                               // 将来使用value.data() 很可能为空

// 确保字符串不被销毁的正确做法：value用于储存，id用于比较/查询
struct tag {
    entt::id_type id;
    std::string value;
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
    // hashed_string可以隐式转换为id_type，既可以赋值...
    registry.emplace<tag>(player, "player"_hs, "player");
    registry.emplace<tag>(enemy, "enemy"_hs, "enemy");
    
    spdlog::info("=== 使用哈希字符串标签 ===");

    // 2. 通过视图遍历并识别实体
    auto view = registry.view<const tag, const position>();
    
    view.each([](const auto& entity_tag, const auto& pos) {
        // 我们可以直接比较哈希值（...也可以比较）
        if (entity_tag.id == "player"_hs) {
            spdlog::info("找到玩家，位置: ({}, {})", pos.x, pos.y);
        }

        if (entity_tag.id == "enemy"_hs) {
            spdlog::info("找到敌人，位置: ({}, {})", pos.x, pos.y);
        }
    });

    // 3. 哈希值与原始值
    auto player_tag = registry.get<tag>(player);
    // spdlog::info("玩家标签的哈希值: {}", player_tag.value.value());
    // spdlog::info("玩家标签的原始文本: {}", player_tag.value.data());
    spdlog::info("玩家标签的哈希值: {}", player_tag.id);
    spdlog::info("玩家标签的原始文本: {}", player_tag.value);

    return 0;
}