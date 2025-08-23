#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>

// 这是一个普通的全局函数，可以作为监听器。
// 它的签名必须与信号的签名兼容。
void free_function_listener(int value, const std::string& message) {
    spdlog::info("[全局函数] 接收到信号: 值 = {}, 消息 = '{}'", value, message);
}

// 这是一个监听器类，我们将把它的成员函数连接到信号上。
class MyListener {
public:
    void on_value_changed(int value, const std::string& message) {
        spdlog::info("[成员函数] 接收到信号: 值 = {}, 消息 = '{}'", value, message);
        member_data++; // 证明我们调用的是特定实例的成员函数
    }
    
    int member_data = 0;
};


int main() {
    // 1. 定义一个信号
    // sigh 的模板参数是它所代表的函数签名。
    // 这个信号可以连接任何接受 (int, const std::string&) 参数的函数。
    entt::sigh<void(int, const std::string&)> on_change;

    // 2. 创建一个 Sink 来管理连接
    // sink 是与信号绑定的临时对象，用于连接/断开监听器。
    entt::sink sink{on_change};
    
    // 创建一个监听器实例
    MyListener listener_instance;

    spdlog::info("=== 连接监听器 ===");
    
    // 3. 连接不同类型的回调函数
    // a) 连接一个全局函数
    sink.connect<&free_function_listener>();
    
    // b) 连接一个类的成员函数，需要提供类的实例
    sink.connect<&MyListener::on_value_changed>(listener_instance);

    // c) 连接一个 Lambda (注意：只有无捕获的 lambda 能被模板推导)
    sink.connect<[](int value, const std::string& message){
        spdlog::info("[Lambda] 接收到信号: 值 = {}, 消息 = '{}'", value, message);
    }>();

    spdlog::info("当前已连接 {} 个监听器。", on_change.size());

    // 4. 发布信号
    // 使用 publish() 来触发信号，所有已连接的监听器都会被调用。
    spdlog::info("\n=== 第一次发布信号 ===");
    on_change.publish(42, "Hello World");

    spdlog::info("Listener 实例内部数据: {}", listener_instance.member_data);
    
    // 5. 断开一个监听器
    spdlog::info("\n=== 断开成员函数监听器 ===");
    sink.disconnect<&MyListener::on_value_changed>(listener_instance);
    
    spdlog::info("当前已连接 {} 个监听器。", on_change.size());
    
    // 6. 再次发布信号
    spdlog::info("\n=== 第二次发布信号 ===");
    on_change.publish(99, "Goodbye");

    spdlog::info("Listener 实例内部数据: {}", listener_instance.member_data);

    return 0;
}