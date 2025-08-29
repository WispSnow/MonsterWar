#pragma once
#include "../../engine/scene/scene.h"

namespace game::scene {

class GameScene final: public engine::scene::Scene {
public:
    GameScene(engine::core::Context& context);
    ~GameScene();

    void init() override;
    void clean() override;

private:
    // --- 测试输入回调事件 (场景切换测试) ---
    int scene_num_{0};
    void onReplace();
    void onPush();
    void onPop();
    void onQuit();

};

} // namespace game::scene