#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <array>
#include <variant>
#include <SDL3/SDL_render.h>
#include <glm/vec2.hpp>
#include <entt/signal/sigh.hpp>

namespace engine::core {
    class Config;
}

namespace engine::input {

/**
 * @brief 动作状态枚举, 除了表示状态外，还将用于函数数组索引(0~2)
 */
enum class ActionState {
    PRESSED,    // 动作在本帧刚刚被按下
    HELD,       // 动作被持续按下
    RELEASED,   // 动作在本帧刚刚被释放
    INACTIVE    // 动作未激活 (放在最后，不占用数组索引)
};

/**
 * @brief 输入管理器类，负责处理输入事件和动作状态。
 * 
 * 该类管理输入事件，将按键转换为动作状态，并提供查询动作状态的功能。
 * 它还处理鼠标位置的逻辑坐标转换。
 */
class InputManager final {
private:
    SDL_Renderer* sdl_renderer_;                                            ///< @brief 用于获取逻辑坐标的 SDL_Renderer 指针

    /** @brief 核心数据结构: 存储动作名称函数列表的映射
     * 
     * @note 每个动作有3个状态: PRESSED, HELD, RELEASED，每个状态对应一个回调函数
     * @note 绑定动作时再插入元素（懒加载），初始化时为空
     */
    std::unordered_map<std::string, std::array<entt::sigh<void()>, 3>> actions_to_func_; 

    /// @brief 存储每个动作的当前状态
    std::unordered_map<std::string, ActionState> action_states_;

    /// @brief 从输入到关联的动作名称列表
    std::unordered_map<std::variant<SDL_Scancode, Uint32>, std::vector<std::string>> input_to_actions_;

    bool should_quit_ = false;                                      ///< @brief 退出标志
    glm::vec2 mouse_position_;                                      ///< @brief 鼠标位置 (针对屏幕坐标)

public:
    /**
     * @brief 构造函数
     * @param sdl_renderer 指向 SDL_Renderer 的指针
     * @param config 配置对象
     * @throws std::runtime_error 如果任一指针为 nullptr。
     */
    InputManager(SDL_Renderer* sdl_renderer, const engine::core::Config* config);

    /**
     * @brief 注册一个动作的回调函数
     * @param action_name_id 动作名称哈希
     * @param action_state 动作状态, 默认为按下瞬间
     * @return 一个 sink 对象，用于注册回调函数
     */
    entt::sink<entt::sigh<void()>> onAction(std::string_view action_name, ActionState action_state = ActionState::PRESSED);


    void update();                                    ///< @brief 更新输入状态，每轮循环最先调用

    // 保留动作状态检查, 提供不同的使用选择
    bool isActionDown(std::string_view action_name) const;        ///< @brief 动作当前是否触发 (持续按下或本帧按下)
    bool isActionPressed(std::string_view action_name) const;     ///< @brief 动作是否在本帧刚刚按下
    bool isActionReleased(std::string_view action_name) const;    ///< @brief 动作是否在本帧刚刚释放

    bool shouldQuit() const;                                         ///< @brief 查询退出状态
    void setShouldQuit(bool should_quit);                            ///< @brief 设置退出状态

    glm::vec2 getMousePosition() const;                              ///< @brief 获取鼠标位置 （屏幕坐标）
    glm::vec2 getLogicalMousePosition() const;                       ///< @brief 获取鼠标位置 （逻辑坐标）

private:
    void processEvent(const SDL_Event& event);                      ///< @brief 处理 SDL 事件（将按键转换为动作状态）
    void initializeMappings(const engine::core::Config* config);    ///< @brief 根据 Config配置初始化映射表

    void updateActionState(std::string_view action_name, bool is_input_active, bool is_repeat_event); ///< @brief 辅助更新动作状态
    SDL_Scancode scancodeFromString(std::string_view key_name);     ///< @brief 将字符串键名转换为 SDL_Scancode
    Uint32 mouseButtonFromString(std::string_view button_name);     ///< @brief 将字符串按钮名转换为 SDL_Button
};

} // namespace engine::input 