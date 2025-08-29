#include "engine/core/game_app.h"
#include "engine/core/context.h"
#include "game/scene/game_scene.h"
#include "engine/utils/events.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_main.h>
#include <entt/signal/dispatcher.hpp>

void setupInitialScene(engine::core::Context& context) {
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto game_scene = std::make_unique<game::scene::GameScene>(context);
    context.getDispatcher().trigger<engine::utils::PushSceneEvent>(engine::utils::PushSceneEvent{std::move(game_scene)});
}


int main(int /* argc */, char* /* argv */[]) {
    spdlog::set_level(spdlog::level::info);

    engine::core::GameApp app;
    app.registerSceneSetup(setupInitialScene);
    app.run();
    return 0;
}