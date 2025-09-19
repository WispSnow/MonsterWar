#include "ui_pressed_state.h"
#include "ui_normal_state.h"
#include "ui_hover_state.h"
#include "../ui_interactive.h"
#include "../../input/input_manager.h"
#include "../../core/context.h"
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace engine::ui::state {

UIPressedState::UIPressedState(engine::ui::UIInteractive* owner)
    : UIState(owner)
{
    owner_->getContext().getInputManager().onAction("mouse_left"_hs, engine::input::ActionState::RELEASED).connect<&UIPressedState::onMouseReleased>(this);
}

UIPressedState::~UIPressedState()
{
    owner_->getContext().getInputManager().onAction("mouse_left"_hs, engine::input::ActionState::RELEASED).disconnect<&UIPressedState::onMouseReleased>(this);
}

void UIPressedState::enter()
{
    owner_->setCurrentImage("pressed"_hs);
    owner_->playSound("ui_click"_hs);
    spdlog::debug("切换到按下状态");
}

bool UIPressedState::onMouseReleased()
{
    auto& input_manager = owner_->getContext().getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (owner_->isPointInside(mouse_pos)) {
        owner_->setNextState(std::make_unique<UIHoverState>(owner_));
        owner_->clicked();
    }
    else {
        owner_->setNextState(std::make_unique<UINormalState>(owner_));
    }
    return true;
}

} // namespace engine::ui::state