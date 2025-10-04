#include "animation_system.h"
#include "../component/animation_component.h"
#include "../component/sprite_component.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

namespace engine::system {

    AnimationSystem::AnimationSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<engine::utils::PlayAnimationEvent>().connect<&AnimationSystem::onPlayAnimationEvent>(this);
}

AnimationSystem::~AnimationSystem() {
    dispatcher_.disconnect(this);
}

void AnimationSystem::update(float dt) {
    auto view = registry_.view<engine::component::AnimationComponent, engine::component::SpriteComponent>();
    for (auto entity : view) {
        auto& anim_component = view.get<engine::component::AnimationComponent>(entity);
        auto& sprite_component = view.get<engine::component::SpriteComponent>(entity);

        // 如果动画不存在，则跳过
        auto it = anim_component.animations_.find(anim_component.current_animation_id_);
        if (it == anim_component.animations_.end()) {
            continue;
        }

        // 获取当前动画
        auto& current_animation = it->second;
        // 如果没有帧，则跳过
        if (current_animation.frames_.empty()) {
            continue;
        }

        // 更新当前播放时间 (推进计时器)
        anim_component.current_time_ms_ += dt * 1000.0f * anim_component.speed_;

        // 获取当前帧
        const auto& current_frame = current_animation.frames_[anim_component.current_frame_index_];

        // 检查是否需要切换到下一帧
        if (anim_component.current_time_ms_ >= current_frame.duration_ms_) {
            anim_component.current_time_ms_ -= current_frame.duration_ms_;
            anim_component.current_frame_index_++;

            // 处理动画播放完成
            if (anim_component.current_frame_index_ >= current_animation.frames_.size()) {
                if (current_animation.loop_) {
                    anim_component.current_frame_index_ = 0;
                } else {
                    // 动画播放完毕且不循环，停在最后一帧
                    anim_component.current_frame_index_ = current_animation.frames_.size() - 1;
                }
            }
        }
        
        // 更新 SpriteComponent 的源矩形 （根据当前动画帧的源矩形信息）
        const auto& next_frame = current_animation.frames_[anim_component.current_frame_index_];
        sprite_component.sprite_.src_rect_ = next_frame.src_rect_;
    }
}

void AnimationSystem::onPlayAnimationEvent(const engine::utils::PlayAnimationEvent& event) {
    // 使用try_get方法来安全获取可能存在的组件。如果不存在则返回nullptr
    if (auto anim = registry_.try_get<engine::component::AnimationComponent>(event.entity_); anim) {
        anim->current_animation_id_ = event.animation_id_;      // 替换动画ID
        anim->current_frame_index_ = 0;
        anim->current_time_ms_ = 0.0f;
        anim->animations_.at(event.animation_id_).loop_ = event.loop_;
    }
}

} // namespace engine::system