#pragma once
#include <memory>

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

} // namespace engine::utils