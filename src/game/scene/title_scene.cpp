#include "title_scene.h"
#include "game_scene.h"
#include "../data/ui_config.h"
#include "../data/session_data.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/core/time.h"
#include "../../engine/core/game_state.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/utils/events.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/loader/level_loader.h"
#include "../../engine/loader/basic_entity_builder.h"
#include "../system/debug_ui_system.h"
#include <spdlog/spdlog.h>
#include <entt/entity/registry.hpp>

using namespace entt::literals;

namespace game::scene {

TitleScene::TitleScene(engine::core::Context& context,
    std::shared_ptr<game::factory::BlueprintManager> blueprint_manager,
    std::shared_ptr<game::data::SessionData> session_data,
    std::shared_ptr<game::data::UIConfig> ui_config, 
    std::shared_ptr<game::data::LevelConfig> level_config)
    : engine::scene::Scene("TitleScene", context), 
    blueprint_manager_(std::move(blueprint_manager)),
    session_data_(std::move(session_data)),
    ui_config_(std::move(ui_config)), 
    level_config_(std::move(level_config)) {
}

TitleScene::~TitleScene() = default;

bool TitleScene::init() {
    if (!initSessionData())         { spdlog::error("Failed to initialize session_data_"); return false; }
    if (!initLevelConfig())         { spdlog::error("Failed to initialize level config"); return false; }
    if (!initBlueprintManager())    { spdlog::error("Failed to initialize blueprint manager"); return false; }
    if (!initUIConfig())            { spdlog::error("Failed to initialize UI config"); return false; }
    if (!loadTitleLevel())          { spdlog::error("Failed to load level"); return false; }
    if (!initSystems())             { spdlog::error("Failed to initialize systems"); return false; }
    if (!initRegistryContext())     { spdlog::error("Failed to initialize registry context"); return false; }
    if (!initUI())                  { spdlog::error("Failed to initialize UI"); return false; }

    context_.getGameState().setState(engine::core::State::Title);
    context_.getTime().setTimeScale(1.0f);      // Reset game speed

    context_.getAudioPlayer().playMusic("title_bgm"_hs);    // Set title bgm
    
    return engine::scene::Scene::init();
}

void TitleScene::update(float delta_time) {
    engine::scene::Scene::update(delta_time);
    animation_system_->update(delta_time);
    movement_system_->update(registry_, delta_time);
    ysort_system_->update(registry_);
}

void TitleScene::render() {
    auto& renderer = context_.getRenderer();
    auto& camera = context_.getCamera();

    render_system_->update(registry_, renderer, camera);

    engine::scene::Scene::render();
    debug_ui_system_->updateTitle(*this);
}

bool TitleScene::initSessionData() {
    if (!session_data_) {
        session_data_ = std::make_shared<game::data::SessionData>();
        if (!session_data_->loadDefaultData()) {
            spdlog::error("Failed to initialize session_data_");
            return false;
        }
    }
    return true;
}

bool TitleScene::initLevelConfig() {
    if (!level_config_) {
        level_config_ = std::make_shared<game::data::LevelConfig>();
        if (!level_config_->loadFromFile("assets/data/level_config.json")) {
            spdlog::error("Failed to load level config");
            return false;
        }
    }
    return true;
}

bool TitleScene::initBlueprintManager() {
    if (!blueprint_manager_) {
        blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
        if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
            !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
            !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json") ||
            !blueprint_manager_->loadEffectBlueprints("assets/data/effect_data.json") ||
            !blueprint_manager_->loadSkillBlueprints("assets/data/skill_data.json")) {
            spdlog::error("Failed to load blueprints");
            return false;
        }
    }
    return true;
}

bool TitleScene::initUIConfig() {
    if (!ui_config_) {
        ui_config_ = std::make_shared<game::data::UIConfig>();
        if (!ui_config_->loadFromFile("assets/data/ui_config.json")) {
            spdlog::error("Failed to load UI config");
            return false;
        }
    }
    return true;
}


bool TitleScene::loadTitleLevel() {
    engine::loader::LevelLoader level_loader;
    if (!level_loader.loadLevel("assets/maps/title.tmj", this)) {
        spdlog::error("Failed to load title level");
        return false;
    }
    return true;
}

bool TitleScene::initSystems() {
    // 初始化系统
    auto& dispatcher = context_.getDispatcher();
    debug_ui_system_ = std::make_unique<game::system::DebugUISystem>(registry_, context_);
    render_system_ = std::make_unique<engine::system::RenderSystem>();
    ysort_system_ = std::make_unique<engine::system::YSortSystem>();
    animation_system_ = std::make_unique<engine::system::AnimationSystem>(registry_, dispatcher);
    movement_system_ = std::make_unique<engine::system::MovementSystem>();
    return true;
}

bool TitleScene::initRegistryContext() {
    // 让注册表存储一些数据类型实例作为上下文，方便使用
    registry_.ctx().emplace<std::shared_ptr<game::data::SessionData>>(session_data_);
    registry_.ctx().emplace<std::shared_ptr<game::factory::BlueprintManager>>(blueprint_manager_);
    registry_.ctx().emplace<std::shared_ptr<game::data::UIConfig>>(ui_config_);
    return true;
}

bool TitleScene::initUI() {
    auto window_size = context_.getGameState().getLogicalSize();
    if (!ui_manager_->init(window_size)) return false;

    /* 先用ImGui实现UI，未来再使用游戏内UI */
    return true;
}

void TitleScene::onStartGameClick() {
    // 如果数据是读档载入的，有可能已经通关，此时需要进入下一关
    if (session_data_->isLevelClear()) {
        session_data_->setLevelClear(false);
        session_data_->addOneLevel();
    }
    requestReplaceScene(std::make_unique<game::scene::GameScene>(
        context_, 
        blueprint_manager_,
        session_data_,
        ui_config_, 
        level_config_
        )
    );
}

void TitleScene::onConfirmRoleClick() {
    show_unit_info_ = !show_unit_info_;
    /* ImGui for rapid logic, future polish with in-game UI */
}

void TitleScene::onLoadGameClick() {
    show_load_panel_ = !show_load_panel_;
    /* ImGui for rapid logic, future polish with in-game UI */
}

void TitleScene::onQuitClick() {
    quit();
}

}   // namespace game::scene