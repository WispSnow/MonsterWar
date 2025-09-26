#include "debug_ui_system.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/render/renderer.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <entt/entity/registry.hpp>

namespace game::system {

DebugUISystem::DebugUISystem(entt::registry& registry, engine::core::Context& context)
    : registry_(registry), context_(context) {}

void DebugUISystem::update() {
    beginFrame();
    renderDemoUI();
    endFrame();
}

void DebugUISystem::beginFrame() {
    // 开始新帧
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // 关闭逻辑分辨率 (ImGui目前对于SDL逻辑分辨率支持不好，所以使用时先关闭)
    if (!context_.getGameState().disableLogicalPresentation()) {
        spdlog::error("关闭逻辑分辨率失败");
    }
}

void DebugUISystem::endFrame() {
    // ImGui: 渲染
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), context_.getRenderer().getSDLRenderer());
    
    // 渲染完成后，打开(恢复)逻辑分辨率
    if (!context_.getGameState().enableLogicalPresentation()) {
        spdlog::error("启用逻辑分辨率失败");
    }
}

void DebugUISystem::renderDemoUI() {
    // --- 中文显示测试 ---
    static float volume_value = 0.5f;
    ImGui::Begin("窗口1");
    ImGui::Text("这是第一个窗口");
    ImGui::SetWindowFontScale(1.5f);
    if (ImGui::Button("按钮1", ImVec2(200, 60))) {
        spdlog::info("按钮1被点击");
    }
    ImGui::SetWindowFontScale(1.0f);
    if (ImGui::SliderFloat("音量", &volume_value, 0.0f, 1.0f)) {
        spdlog::info("音量被调整: {}", volume_value);
    }
    ImGui::End();

    // 显示 ImGui 自带的 Demo 窗口
    ImGui::ShowDemoWindow();
}

} // namespace game::system