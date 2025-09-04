#include "ui_interactive.h"
#include "state/ui_state.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "../audio/audio_player.h"
#include <spdlog/spdlog.h>

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

void UIInteractive::addImage(entt::id_type name_id, engine::render::Image image)
{
    // 可交互UI元素必须有一个size用于交互检测，因此如果参数列表中没有指定，则用图片大小作为size
    if (size_.x == 0.0f && size_.y == 0.0f) {
        size_ = context_.getResourceManager().getTextureSize(image.getTextureId());
    }
    // 添加精灵
    images_.emplace(name_id, std::move(image));
}

void UIInteractive::setImage(entt::id_type name_id)
{
    if (images_.find(name_id) != images_.end()) {
        current_image_id_ = name_id;
    } else {
        spdlog::warn("Image '{}' 未找到", name_id);
    }
}

void UIInteractive::addSound(entt::id_type name_id, entt::hashed_string hashed_path)
{
    // 插入容器
    sounds_.emplace(name_id, hashed_path.value());
    // 载入音效资源
    context_.getResourceManager().loadSound(hashed_path);
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

bool UIInteractive::handleInput(engine::core::Context &context)
{
    if (UIElement::handleInput(context)) {  
        return true;
    }

    // 先更新子节点，再更新自己（状态）
    if (state_ && interactive_) {
        if (auto next_state = state_->handleInput(context); next_state) {
            setState(std::move(next_state));
            return true;
        }
    }
    return false;
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