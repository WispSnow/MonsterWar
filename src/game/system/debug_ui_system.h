#pragma once
#include <entt/entity/fwd.hpp>

namespace engine::core {
    class Context;
}

namespace game::system {

/**
 * @brief 调试 UI 系统，负责显示调试 UI。
 * 
 * @note 调试UI的主要目的是方便debug，并快速开发UI原型。
 * @note 游戏正式发布时往往会删除，因此不需要过度设计。
 */
class DebugUISystem {
    entt::registry& registry_;
    engine::core::Context& context_;
public:
    DebugUISystem(entt::registry& registry, engine::core::Context& context);

    // ImGui 步骤3: 一轮循环内，ImGui 需要做的操作（逻辑+渲染）
    void update();

private:
    // 封装开始、结束帧的方法
    void beginFrame();
    void endFrame();

    // 封装每个UI显示模块
    void renderDemoUI();

};

} // namespace game::system