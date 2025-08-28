#pragma once
#include "../../engine/scene/scene.h"

namespace game::scene {

class GameScene final: public engine::scene::Scene {
public:
    GameScene(engine::core::Context& context, engine::scene::SceneManager& scene_manager);
    ~GameScene();

    void init() override;
    void clean() override;

private:
    // --- 测试输入回调事件 ---
    void onAttack();
    void onJump();

};

} // namespace game::scene