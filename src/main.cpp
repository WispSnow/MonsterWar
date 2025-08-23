#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>

struct position { float x; float y; };
struct velocity { float dx; float dy; };
struct tag { entt::id_type id; std::string value; };

// 上下文变量
struct game_state {
    int score = 0;
};

// 1. 定义事件
// 事件通常是简单的结构体，用于携带数据。
// 这个事件携带了被消灭的敌人的信息。
struct enemy_destroyed_event {
    entt::entity enemy_entity;
    // 可以在此添加更多信息，比如敌人类型、掉落物品等
};

// 2. 创建一个监听器，监听器可以是一个独立的类或一个函数。
// 这里使用类来组织逻辑。
class ScoreSystem {
    entt::registry& registry;   // 需要 registry 的引用，以便获取上下文
public:
    ScoreSystem(entt::registry& reg) : registry(reg) {}

    // 这是事件处理函数。当接收到 enemy_destroyed_event 时，这个函数会被调用。
    void on_enemy_destroyed(const enemy_destroyed_event& event) {
        // 从上下文中获取游戏状态并增加分数
        auto& state = registry.ctx().get<game_state>();
        state.score += 10;

        spdlog::info("杀死敌人 {}，分数增加！当前分数: {}", entt::to_integral(event.enemy_entity), state.score);
            /* entt::entity 的底层是 entt::id_type，即 uint32_t，但不可直接当成整数用（保证类型安全），
                                                    可以用 entt::to_integral 显式转换为 uint32_t */
    }
};

// 另一种监听器，直接使用函数
void dummy_listener(const enemy_destroyed_event& event) {
    spdlog::info("DummyListener 收到事件：敌人 {} 被摧毁！", entt::to_integral(event.enemy_entity));
}

int main() {
    using namespace entt::literals;
    entt::registry registry;

    // 初始化上下文变量
    registry.ctx().emplace<game_state>();

    // 3. 创建事件分发器 (dispatcher) 和监听器实例
    entt::dispatcher dispatcher{};
    ScoreSystem score_system(registry);

    // 4. 连接监听器到分发器
    // dispatcher.sink<EventType>().connect<&Class::MemberFunction>(instance)
    // 这行代码告诉分发器：当有 enemy_destroyed_event 类型的事件时，
    // 调用 score_system 实例的 on_enemy_destroyed 方法。
    dispatcher.sink<enemy_destroyed_event>().connect<&ScoreSystem::on_enemy_destroyed>(score_system);

    // 同一个事件可以连接多个函数（注意调用的顺序，后进先调）
    dispatcher.sink<enemy_destroyed_event>().connect<&dummy_listener>();

    // 创建实体
    entt::entity enemy_to_destroy = registry.create();
    registry.emplace<tag>(enemy_to_destroy, "enemy"_hs, "enemy");

    spdlog::info("=== 游戏进行中 ===");
    spdlog::info("初始分数: {}", registry.ctx().get<game_state>().score);
    
    // ... 战斗发生 ...
    spdlog::info("玩家摧毁了敌人 {}!", static_cast<uint32_t>(enemy_to_destroy));
    registry.destroy(enemy_to_destroy); // 从 registry 中移除实体

    // 5. 发布事件
    // 使用 enqueue 来将事件放入队列，通常在游戏循环的末尾统一处理。 (trigger 会立即触发)
    dispatcher.enqueue(enemy_destroyed_event{enemy_to_destroy});

    // 6. 更新分发器
    // 在游戏循环的某个固定点（比如末尾），调用 update() 来处理队列中的所有事件。
    // 这时，所有连接的监听器函数才会被调用。
    dispatcher.update();
    
    spdlog::info("=== 游戏循环结束 ===");
    spdlog::info("最终分数: {}", registry.ctx().get<game_state>().score);

    return 0;
}