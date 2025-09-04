#pragma once
#include "../../engine/utils/math.h"
#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <unordered_map>
#include <vector>

namespace engine::component {

/**
 * @brief 动画帧数据结构
 * 
 * 包含帧源矩形和帧间隔（毫秒）。
 */
struct AnimationFrame {
    engine::utils::Rect src_rect_{};        ///< @brief 帧源矩形
    float duration_ms_{100.0f};             ///< @brief 帧间隔（毫秒）
    AnimationFrame(engine::utils::Rect src_rect, float duration_ms = 100.0f)
     : src_rect_(std::move(src_rect)), duration_ms_(duration_ms) {}
};

/**
 * @brief 动画数据结构
 * 
 * 包含动画名称、帧列表、总时长、当前播放时间、是否循环等属性。
 */
struct Animation {
    std::vector<AnimationFrame> frames_;    ///< @brief 动画帧
    float total_duration_ms_{};             ///< @brief 动画总时长（毫秒）
    bool loop_{true};                       ///< @brief 是否循环

    /**
     * @brief 构造函数
     * @param name 动画名称
     * @param frames 动画帧
     * @param loop 是否循环，默认true
     */
    Animation(std::vector<AnimationFrame> frames, 
              bool loop = true) : 
              frames_(std::move(frames)), 
              loop_(loop) {
        // 计算动画总时长 (总时长 = 所有帧时长之和)
        total_duration_ms_ = 0.0f;
        for (const auto& frame : frames_) {
            total_duration_ms_ += frame.duration_ms_;
        }
    }
};

/**
 * @brief 动画组件
 * 
 * 包含动画名称、帧列表、总时长、当前播放时间、是否循环等属性。
 */
struct AnimationComponent {
    std::unordered_map<entt::id_type, Animation> animations_;   ///< @brief 动画集合
    entt::id_type current_animation_id_{entt::null};            ///< @brief 当前播放的动画名称
    size_t current_frame_index_{};                              ///< @brief 当前播放的帧索引
    float current_time_ms_{};                                   ///< @brief 当前播放时间（毫秒）
    float speed_{1.0f};                                         ///< @brief 播放速度

    /**
     * @brief 构造函数
     * @param animations 动画集合
     * @param current_animation_name 当前播放的动画名称
     * @param current_frame_index 当前播放的帧索引
     * @param current_time_ms 当前播放时间（毫秒）
     * @param speed 播放速度
     */
    AnimationComponent(std::unordered_map<entt::id_type, Animation> animations,
                       entt::id_type current_animation_id,
                       size_t current_frame_index = 0,
                       float current_time_ms = 0.0f,
                       float speed = 1.0f) : 
                       animations_(std::move(animations)),
                       current_animation_id_(current_animation_id),
                       current_frame_index_(current_frame_index),
                       current_time_ms_(current_time_ms),
                       speed_(speed) {}
};

}