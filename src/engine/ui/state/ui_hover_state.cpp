#include "ui_hover_state.h"
#include "ui_normal_state.h"
#include "ui_pressed_state.h"
#include "../ui_interactive.h"
#include "../../input/input_manager.h"
#include "../../core/context.h"
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace engine::ui::state {

UIHoverState::UIHoverState(engine::ui::UIInteractive* owner)
    : UIState(owner)
{
    owner_->getContext().getInputManager().onAction("mouse_left"_hs).connect<&UIHoverState::onMousePressed>(this);
}

UIHoverState::~UIHoverState()
{
    owner_->getContext().getInputManager().onAction("mouse_left"_hs).disconnect<&UIHoverState::onMousePressed>(this);
}

void UIHoverState::enter()
{
    owner_->setCurrentImage("hover"_hs);
    owner_->hover_enter();
    spdlog::debug("切换到悬停状态");
}

void UIHoverState::update(float, engine::core::Context& context)
{
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (!owner_->isPointInside(mouse_pos)) {                // 如果鼠标不在UI元素内，则设置正常状态
        owner_->hover_leave();
        owner_->setNextState(std::make_unique<UINormalState>(owner_));
    }
}

bool UIHoverState::onMousePressed()
{
    owner_->setNextState(std::make_unique<UIPressedState>(owner_));
    return true;
}

} // namespace engine::ui::state