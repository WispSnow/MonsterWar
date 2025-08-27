#pragma once
#include "../../engine/scene/scene.h"

namespace game::scene {

class GameScene : public engine::scene::Scene {
public:
    GameScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager);
    ~GameScene();

};

} // namespace game::scene