#include "game_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include <entt/signal/sigh.hpp>
#include <spdlog/spdlog.h>

namespace game::scene {

GameScene::GameScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager)
    : engine::scene::Scene("GameScene", context, scene_manager) {
}

GameScene::~GameScene() {
}

void GameScene::init() {
    // 注册输入回调事件 (J,K 键)
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("attack").connect<&GameScene::onAttack>(this);       // 默认绑定PRESSED
    input_manager.onAction("jump", engine::input::ActionState::RELEASED).connect<&GameScene::onJump>(this);
}

void GameScene::clean() {
    // 断开输入回调事件 (谁连接，谁负责断开)
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("attack").disconnect<&GameScene::onAttack>(this);
    input_manager.onAction("jump", engine::input::ActionState::RELEASED).disconnect<&GameScene::onJump>(this);
}

void GameScene::onAttack() {
    spdlog::info("onAttack");
}

void GameScene::onJump() {
    spdlog::info("onJump");
}

} // namespace game::scene
