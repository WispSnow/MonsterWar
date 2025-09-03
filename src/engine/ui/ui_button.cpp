#include "ui_button.h"
#include "state/ui_normal_state.h"
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace engine::ui {
UIButton::UIButton(engine::core::Context& context,
                   std::string_view normal_sprite_id, 
                   std::string_view hover_sprite_id, 
                   std::string_view pressed_sprite_id, 
                   glm::vec2 position, 
                   glm::vec2 size, 
                   std::function<void()> callback)
    : UIInteractive(context, std::move(position), std::move(size)), callback_(std::move(callback))
{
    addSprite("normal"_hs, engine::render::Sprite(normal_sprite_id));
    addSprite("hover"_hs, engine::render::Sprite(hover_sprite_id));
    addSprite("pressed"_hs, engine::render::Sprite(pressed_sprite_id));

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