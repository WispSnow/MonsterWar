#pragma once
#include "ui_state.h"

namespace engine::ui::state {

/**
 * @brief 悬停状态
 *
 * 当鼠标悬停在UI元素上时，会切换到该状态。
 */
class UIHoverState final: public UIState {
    friend class engine::ui::UIInteractive;
public:
    UIHoverState(engine::ui::UIInteractive* owner);
    ~UIHoverState();

private:
    void enter() override;
    void update(float delta_time, engine::core::Context& context) override;

    bool onMousePressed();  ///< @brief 鼠标按下回调函数 (不再使用轮询“isActionPressed”)
};

} // namespace engine::ui::state