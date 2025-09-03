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
    // --- 测试资源管理器 ---
    void testResourceManager();

};

} // namespace game::scene