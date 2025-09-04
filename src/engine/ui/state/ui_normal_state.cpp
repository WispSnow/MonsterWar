#include "ui_normal_state.h"
#include "ui_hover_state.h"
#include "../ui_interactive.h"
#include "../../input/input_manager.h"
#include "../../core/context.h"
#include "../../audio/audio_player.h"
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace engine::ui::state {

void UINormalState::enter()
{
    owner_->setImage("normal"_hs);
    spdlog::debug("切换到正常状态");
}

std::unique_ptr<UIState> UINormalState::handleInput(engine::core::Context& context)
{
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (owner_->isPointInside(mouse_pos)) {         // 如果鼠标在UI元素内，则切换到悬停状态
        owner_->playSound("hover"_hs);
        return std::make_unique<engine::ui::state::UIHoverState>(owner_);
    }
    return nullptr;
}

} // namespace engine::ui::state