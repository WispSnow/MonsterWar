#include "game_scene.h"

namespace game::scene {

GameScene::GameScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager)
    : Scene("GameScene", context, scene_manager) {
}

GameScene::~GameScene() {
}

} // namespace game::scene
