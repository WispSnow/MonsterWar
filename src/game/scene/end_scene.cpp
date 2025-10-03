#include "end_scene.h"
#include "title_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/utils/events.h"
#include "../../engine/loader/level_loader.h"
#include "../../engine/loader/basic_entity_builder.h"
#include "../system/debug_ui_system.h"
#include <spdlog/spdlog.h>
#include <entt/entity/registry.hpp>

using namespace entt::literals;

namespace game::scene {

EndScene::EndScene(engine::core::Context& context, bool is_win)
    : engine::scene::Scene("EndScene", context),
    is_win_(is_win) {
    debug_ui_system_ = std::make_unique<game::system::DebugUISystem>(registry_, context);
}

EndScene::~EndScene() = default;

void EndScene::init() {
    if (is_win_) {
        context_.getAudioPlayer().playMusic("win"_hs, 0);
    } else {
        context_.getAudioPlayer().playMusic("lose"_hs, 0);
    }
    context_.getGameState().setState(engine::core::State::GameOver);
}

void EndScene::render() {
    engine::scene::Scene::render();
    debug_ui_system_->updateEnd(*this);
}

void EndScene::onBackToTitleClick() {
    requestReplaceScene(std::make_unique<game::scene::TitleScene>(context_));
}

void EndScene::onQuitClick() {
    quit();
}

}   // namespace game::scene