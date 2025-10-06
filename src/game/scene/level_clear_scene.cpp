#include "level_clear_scene.h"
#include "game_scene.h"
#include "title_scene.h"
#include "../data/ui_config.h"
#include "../data/session_data.h"
#include "../../engine/ui/ui_manager.h"
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

LevelClearScene::LevelClearScene(engine::core::Context& context, 
    std::shared_ptr<game::factory::BlueprintManager> blueprint_manager,
    std::shared_ptr<game::data::UIConfig> ui_config, 
    std::shared_ptr<game::data::LevelConfig> level_config,
    std::shared_ptr<game::data::SessionData> session_data,
    game::data::GameStats& game_stats)
    : engine::scene::Scene("LevelClearScene", context),
    blueprint_manager_(std::move(blueprint_manager)),
    ui_config_(std::move(ui_config)),
    level_config_(std::move(level_config)),    
    session_data_(std::move(session_data)),
    game_stats_(game_stats) {
        // 直接在构造函数中初始化DebugUI系统
        debug_ui_system_ = std::make_unique<game::system::DebugUISystem>(registry_, context);
    }

LevelClearScene::~LevelClearScene() = default;

bool LevelClearScene::init() {
    if (!ui_config_ || !level_config_ || !session_data_ || !blueprint_manager_) {
        spdlog::error("LevelClearScene: ui_config_, level_config_, session_data_ or blueprint_manager_ 必须有值");
        return false;
    }
    context_.getGameState().setState(engine::core::State::LevelClear);

    registry_.ctx().emplace<std::shared_ptr<game::data::SessionData>>(session_data_);
    registry_.ctx().emplace<std::shared_ptr<game::factory::BlueprintManager>>(blueprint_manager_);
    registry_.ctx().emplace<std::shared_ptr<game::data::UIConfig>>(ui_config_);
    context_.getAudioPlayer().playMusic("win"_hs, 0);
    return engine::scene::Scene::init();
}

void LevelClearScene::render() {
    engine::scene::Scene::render();
    debug_ui_system_->updateLevelClear(*this);
}

void LevelClearScene::onNextLevelClick() {
    session_data_->addOneLevel();
    session_data_->setLevelClear(false);
    requestReplaceScene(std::make_unique<game::scene::GameScene>(
        context_, 
        blueprint_manager_,
        session_data_,
        ui_config_, 
        level_config_)
    );
}

void LevelClearScene::onBackToTitleClick() {
    requestReplaceScene(std::make_unique<game::scene::TitleScene>(context_));
}

void LevelClearScene::onSaveClick() {
    show_save_panel_ = !show_save_panel_;
}


}   // namespace game::scene