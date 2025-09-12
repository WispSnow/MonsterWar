#pragma once
#include <memory>
#include <entt/entity/entity.hpp>

namespace engine::scene {
    class Scene;
}

namespace engine::utils {

struct QuitEvent {};        // 退出事件
struct PopSceneEvent {};    // 弹出场景事件
struct PushSceneEvent {     // 压入场景事件
    std::unique_ptr<engine::scene::Scene> scene; 
};
struct ReplaceSceneEvent {  // 替换场景事件
    std::unique_ptr<engine::scene::Scene> scene; 
};

/// @brief 播放动画事件
struct PlayAnimationEvent {
    entt::entity entity_{entt::null};           ///< @brief 目标实体
    entt::id_type animation_id_{entt::null};    ///< @brief 动画ID
    bool loop_{true};                           ///< @brief 是否循环
};

} // namespace engine::utils