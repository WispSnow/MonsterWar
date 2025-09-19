#include "ui_button.h"
#include "state/ui_normal_state.h"
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace engine::ui {
UIButton::UIButton(engine::core::Context& context,
                   engine::render::Image normal_image, 
                   engine::render::Image hover_image, 
                   engine::render::Image pressed_image, 
                   glm::vec2 position, 
                   glm::vec2 size, 
                   std::function<void()> click_callback,
                   std::function<void()> hover_enter_callback,
                   std::function<void()> hover_leave_callback)
    : UIInteractive(context, std::move(position), std::move(size)), click_callback_(std::move(click_callback)), hover_enter_callback_(std::move(hover_enter_callback)), hover_leave_callback_(std::move(hover_leave_callback))
{
    // 注意正常、悬浮、按下都有默认的键名称，如果需要替换的话则覆盖该键下的值
    addImage("normal"_hs, std::move(normal_image));
    addImage("hover"_hs, std::move(hover_image));
    addImage("pressed"_hs, std::move(pressed_image));

    // 设置默认状态为"normal"
    setState(std::make_unique<engine::ui::state::UINormalState>(this));

    spdlog::trace("UIButton 构造完成");
}

} // namespace engine::ui