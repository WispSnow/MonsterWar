#pragma once
#include "ui_interactive.h"
#include <functional>
#include <utility>

namespace engine::ui {

/**
 * @brief 按钮UI元素
 * 
 * 继承自UIInteractive，用于创建可交互的按钮。
 * 支持三种状态：正常、悬停、按下。
 * 支持回调函数，当按钮被点击时调用。
 */
class UIButton final: public UIInteractive {
private:
    std::function<void()> click_callback_;          ///< @brief 点击回调函数
    std::function<void()> hover_enter_callback_;    ///< @brief 悬停进入回调函数
    std::function<void()> hover_leave_callback_;    ///< @brief 悬停离开回调函数

public:
    /**
     * @brief 构造函数
     * @param normal_image 正常状态的图片Image类
     * @param hover_image 悬停状态的图片Image类
     * @param pressed_image 按下状态的图片Image类
     * @param position 位置
     * @param size 大小
     * @param click_callback 点击回调函数
     * @param hover_enter_callback 悬停进入回调函数
     * @param hover_leave_callback 悬停离开回调函数
     */
    UIButton(engine::core::Context& context,
             engine::render::Image normal_image,
             engine::render::Image hover_image,
             engine::render::Image pressed_image,
             glm::vec2 position = {0.0f, 0.0f}, 
             glm::vec2 size = {0.0f, 0.0f}, 
             std::function<void()> click_callback = nullptr,
             std::function<void()> hover_enter_callback = nullptr,
             std::function<void()> hover_leave_callback = nullptr);
    ~UIButton() override = default;

    // --- 重写基类方法：点击、悬停进入、悬停离开 ---
    void clicked() override { if (click_callback_) click_callback_(); }
    void hover_enter() override { if (hover_enter_callback_) hover_enter_callback_(); }
    void hover_leave() override { if (hover_leave_callback_) hover_leave_callback_(); }

    // --- Setters & Getters ---
    /// @brief 设置点击回调函数
    void setClickCallback(std::function<void()> callback) { click_callback_ = std::move(callback); }   
    /// @brief 设置悬停进入回调函数
    void setHoverEnterCallback(std::function<void()> callback) { hover_enter_callback_ = std::move(callback); }  
    /// @brief 设置悬停离开回调函数
    void setHoverLeaveCallback(std::function<void()> callback) { hover_leave_callback_ = std::move(callback); }

    std::function<void()> getClickCallback() const { return click_callback_; }              ///< @brief 获取点击回调函数
    std::function<void()> getHoverEnterCallback() const { return hover_enter_callback_; }   ///< @brief 获取悬停进入回调函数
    std::function<void()> getHoverLeaveCallback() const { return hover_leave_callback_; }   ///< @brief 获取悬停离开回调函数

};

} // namespace engine::ui