#pragma once
#include "ui_element.h"
#include "state/ui_state.h"
#include "../render/image.h"   // 需要引入头文件而不是前置声明（map容器创建时可能会检查内部元素是否有析构定义）
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <entt/core/hashed_string.hpp>

namespace engine::core {
    class Context;
}

namespace engine::ui {

/**
 * @brief 可交互UI元素的基类,继承自UIElement
 *
 * 定义了可交互UI元素的通用属性和行为。
 * 管理UI状态的切换和交互逻辑。
 * 提供事件处理、更新和渲染的虚方法。
 */
class UIInteractive : public UIElement {
protected:
    engine::core::Context& context_;                        ///< @brief 可交互元素很可能需要其他引擎组件
    std::unique_ptr<engine::ui::state::UIState> state_;     ///< @brief 当前状态
    std::unordered_map<entt::id_type, engine::render::Image> images_;   ///< @brief 精灵集合
    std::unordered_map<entt::id_type, entt::id_type> sounds_;           ///< @brief 音效集合，key为音效名称ID，value为音效ID
    entt::id_type current_image_id_;                        ///< @brief 当前显示的精灵ID
    bool interactive_ = true;                               ///< @brief 是否可交互

public:
    UIInteractive(engine::core::Context& context, glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f});
    ~UIInteractive() override;

    virtual void clicked() {}       ///< @brief 如果有点击事件，则重写该方法

    void addImage(entt::id_type name_id, engine::render::Image image);      ///< @brief 添加精灵
    void setImage(entt::id_type name_id);                                   ///< @brief 设置当前显示的精灵
    void addSound(entt::id_type name_id, entt::hashed_string hashed_path);  ///< @brief 添加音效
    void playSound(entt::id_type name_id);                                  ///< @brief 播放音效
    // --- Getters and Setters ---
    void setState(std::unique_ptr<engine::ui::state::UIState> state);       ///< @brief 设置当前状态
    engine::ui::state::UIState* getState() const { return state_.get(); }   ///< @brief 获取当前状态

    void setInteractive(bool interactive) { interactive_ = interactive; }   ///< @brief 设置是否可交互
    bool isInteractive() const { return interactive_; }                     ///< @brief 获取是否可交互

    // --- 核心方法 ---
    bool handleInput(engine::core::Context& context) override;
    void render(engine::core::Context& context) override;
};

} // namespace engine::ui