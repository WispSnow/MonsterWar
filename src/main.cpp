#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>

// 一个符合委托签名的全局函数
int multiply_by_two(int value) {
    spdlog::info("[全局函数] 被调用，输入: {}", value);
    return value * 2;
}

// 一个普通的类，使用它的成员函数
class Calculator {
    int multiplication_factor;
    
public:
    Calculator(int factor) : multiplication_factor(factor) {}

    int multiply(int value) const {
        spdlog::info("[成员函数] 被调用，输入: {}", value);
        return value * multiplication_factor;
    }
};


int main() {
    // 1. 定义一个委托
    // 模板参数是委托期望的函数签名：一个接受 int 并返回 int 的函数。
    entt::delegate<int(int)> my_delegate;

    // 2. 检查委托是否有效
    // 刚创建的委托是空的，调用它会导致未定义行为。
    if (!my_delegate) {
        spdlog::info("委托当前是空的。");
    }

    // 3. 连接一个全局函数
    spdlog::info("\n=== 连接全局函数 ===");
    my_delegate.connect<&multiply_by_two>();

    if (my_delegate) {
        // 调用委托，就像调用一个普通函数
        int result = my_delegate(10);
        spdlog::info("委托调用的结果: {}", result);
    }

    // 4. 连接一个成员函数
    // 连接成员函数需要一个类的实例。
    spdlog::info("\n=== 连接成员函数 ===");
    Calculator my_calculator(5);
    my_delegate.connect<&Calculator::multiply>(my_calculator);

    if (my_delegate) {
        int result = my_delegate(10);
        spdlog::info("委托调用的结果: {}", result);
    }
    
    // 5. 重置委托
    // reset() 会清空委托，使其变回无效状态。
    spdlog::info("\n=== 重置委托 ===");
    my_delegate.reset();

    if (!my_delegate) {
        spdlog::info("委托已被重置为空。");
    }

    // 6. 使用构造函数直接连接 (便捷方式)
    Calculator other_calculator(3);
    entt::delegate<int(int)> another_delegate{entt::connect_arg<&Calculator::multiply>, other_calculator};
    
    spdlog::info("\n=== 使用构造函数连接 ===");
    int result = another_delegate(10);
    spdlog::info("新委托调用的结果: {}", result);


    return 0;
}