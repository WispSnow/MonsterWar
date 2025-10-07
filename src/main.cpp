#include "engine/core/game_app.h"
#include "engine/core/context.h"
#include "game/scene/title_scene.h"
#include "engine/utils/events.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_main.h>
#include <entt/signal/dispatcher.hpp>

// 只在 Windows 平台上包含 Windows.h
#ifdef _WIN32
#include <Windows.h>
#endif

// 在程序开始时设置控制台编码
void initialize_environment() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void setupInitialScene(engine::core::Context& context) {
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto title_scene = std::make_unique<game::scene::TitleScene>(context);
    context.getDispatcher().trigger<engine::utils::PushSceneEvent>(engine::utils::PushSceneEvent{std::move(title_scene)});
}


int main(int /* argc */, char* /* argv */[]) {
    initialize_environment();
    spdlog::set_level(spdlog::level::info);

    engine::core::GameApp app;
    app.registerSceneSetup(setupInitialScene);
    app.run();
    return 0;
}