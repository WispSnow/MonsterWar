#include "ui_interactive.h"
#include "state/ui_state.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "../audio/audio_player.h"
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace engine::ui {

UIInteractive::~UIInteractive() = default;

UIInteractive::UIInteractive(engine::core::Context &context, glm::vec2 position, glm::vec2 size)
    : UIElement(std::move(position), std::move(size)), context_(context)
{
    spdlog::trace("UIInteractive 构造完成");
}

void UIInteractive::setState(std::unique_ptr<engine::ui::state::UIState> state)
{
    if (!state) {
        spdlog::warn("尝试设置空的状态！");
        return;
    }

    state_ = std::move(state);
    state_->enter();
}

void UIInteractive::setNextState(std::unique_ptr<engine::ui::state::UIState> state)
{
    next_state_ = std::move(state);
}

void UIInteractive::addImage(entt::id_type name_id, engine::render::Image image)
{
    // 可交互UI元素必须有一个size用于交互检测，因此如果参数列表中没有指定，则用图片大小作为size
    if (size_.x == 0.0f && size_.y == 0.0f) {
        size_ = context_.getResourceManager().getTextureSize(image.getTextureId());
    }
    // 添加图片 (如果name_id已存在，则替换)
    images_.insert_or_assign(name_id, std::move(image));
}

void UIInteractive::setCurrentImage(entt::id_type name_id)
{
    if (images_.find(name_id) != images_.end()) {
        current_image_id_ = name_id;
    } else {
        spdlog::warn("Image '{}' 未找到", name_id);
    }
}

void UIInteractive::setHoverSound(entt::id_type id, std::string_view path)
{
    context_.getResourceManager().loadSound(id, path);    // 确保音效资源被加载
    sounds_.emplace("ui_hover"_hs, id);
}

void UIInteractive::setClickSound(entt::id_type id, std::string_view path)
{
    context_.getResourceManager().loadSound(id, path);    // 确保音效资源被加载
    sounds_.emplace("ui_click"_hs, id);
}

void UIInteractive::playSound(entt::id_type name_id)
{
    // 先尝试在自定义sounds_中查找（map的值）
    if (auto it = sounds_.find(name_id); it != sounds_.end()) {
        if (context_.getAudioPlayer().playSound(it->second) == -1) {
            spdlog::warn("Sound '{}' 未找到或无法播放", name_id);
        }
    } 
    // 如果自定义sounds_中没有找到，则使用默认音效（map的键）
    else {
        if (context_.getAudioPlayer().playSound(name_id) == -1) {
            spdlog::error("Sound '{}' 未找到或无法播放", name_id);
        }
    }
}

void UIInteractive::update(float delta_time, engine::core::Context &context)
{
    // 先更新子节点
    UIElement::update(delta_time, context);

    // 再更新自己（状态）
    if (state_ && interactive_) {
        if (next_state_) {
            setState(std::move(next_state_));
            next_state_.reset();
        } 
        state_->update(delta_time, context);
    }
}

void UIInteractive::render(engine::core::Context &context)
{
    if (!visible_ ) return;

    // 先渲染自身
    context.getRenderer().drawUIImage(images_[current_image_id_], getScreenPosition(), size_);

    // 再渲染子元素（调用基类方法）
    UIElement::render(context);
}

} // namespace engine::ui