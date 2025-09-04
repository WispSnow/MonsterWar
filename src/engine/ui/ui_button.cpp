#include "ui_button.h"
#include "state/ui_normal_state.h"
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace engine::ui {
UIButton::UIButton(engine::core::Context& context,
                   std::string_view normal_image_id, 
                   std::string_view hover_image_id, 
                   std::string_view pressed_image_id, 
                   glm::vec2 position, 
                   glm::vec2 size, 
                   std::function<void()> callback)
    : UIInteractive(context, std::move(position), std::move(size)), callback_(std::move(callback))
{
    addImage("normal"_hs, engine::render::Image(normal_image_id));
    addImage("hover"_hs, engine::render::Image(hover_image_id));
    addImage("pressed"_hs, engine::render::Image(pressed_image_id));

    // 设置默认状态为"normal"
    setState(std::make_unique<engine::ui::state::UINormalState>(this));

    // 设置默认音效
    addSound("hover"_hs, "assets/audio/button_hover.wav"_hs);
    addSound("pressed"_hs, "assets/audio/button_click.wav"_hs);
    spdlog::trace("UIButton 构造完成");
}

void UIButton::clicked()
{
    if (callback_) callback_();
}

} // namespace engine::ui