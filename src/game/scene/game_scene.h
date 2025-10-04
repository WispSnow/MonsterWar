#pragma once
#include "../../engine/scene/scene.h"
#include "../../engine/system/fwd.h"
#include <memory>

namespace game::scene {

class GameScene final: public engine::scene::Scene {
private:
    std::unique_ptr<engine::system::RenderSystem> render_system_;
    std::unique_ptr<engine::system::MovementSystem> movement_system_;
    std::unique_ptr<engine::system::AnimationSystem> animation_system_;

public:
    GameScene(engine::core::Context& context);
    ~GameScene();

    void init() override;
    void update(float delta_time) override;
    void render() override;
    void clean() override;

private:
    // --- 测试资源管理器 ---
    void testResourceManager();
    // --- 测试ECS ---
    void testECS();

};

} // namespace game::scene