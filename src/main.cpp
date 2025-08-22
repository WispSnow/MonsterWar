#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>

using namespace entt::literals;

void add_context(entt::registry& registry) {
    // 往上下文中添加一些数据（注意默认情况下，每种类型只能添加一个）
    registry.ctx().emplace<bool>(true);
    registry.ctx().emplace<glm::vec2>(1.0f, 2.0f);
    registry.ctx().emplace<int>(42);
    registry.ctx().emplace<std::string>("hello");
    // 如果要添加同种类型的数据，可以使用 emplace_as + 哈希标签
    registry.ctx().emplace_as<std::string>("world"_hs, "world");
}

void get_context(entt::registry& registry) {
    // 从上下文中获取数据
    bool bool_value = registry.ctx().get<bool>();
    glm::vec2 vec2_value = registry.ctx().get<glm::vec2>();
    int int_value = registry.ctx().get<int>();
    std::string string_value = registry.ctx().get<std::string>();
    auto new_string = registry.ctx().get<std::string>("world"_hs);

    spdlog::info("bool 值: {}", bool_value);
    spdlog::info("vec2 值: ({}, {})", vec2_value.x, vec2_value.y);
    spdlog::info("int 值: {}", int_value);
    spdlog::info("string 值: {}", string_value);
    spdlog::info("new_string 值: {}", new_string);
}

int main() {
    // 每个 registry 实例都包含一个上下文，你可以把它看作是一个类型安全的、与注册表绑定的全局变量容器。
    entt::registry registry;

    add_context(registry);
    get_context(registry);

    registry.clear();

    return 0;
}