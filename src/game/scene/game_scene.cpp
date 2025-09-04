#include "game_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/resource/resource_manager.h"
#include "../../engine/render/text_renderer.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_label.h"
#include <unordered_map>
#include <entt/core/hashed_string.hpp>
#include <entt/signal/sigh.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::scene {

GameScene::GameScene(engine::core::Context& context)
    : engine::scene::Scene("GameScene", context) {

    // 初始化系统
    render_system_ = std::make_unique<engine::system::RenderSystem>();
    movement_system_ = std::make_unique<engine::system::MovementSystem>();
    animation_system_ = std::make_unique<engine::system::AnimationSystem>();

    spdlog::info("GameScene 构造完成");
}

GameScene::~GameScene() {
}

void GameScene::init() {
    // 测试资源管理器
    testResourceManager();
    // 测试ECS
    testECS();
    
    Scene::init();
}

void GameScene::update(float delta_time) {
    movement_system_->update(registry_, delta_time);
    animation_system_->update(registry_, delta_time);

    Scene::update(delta_time);
}

void GameScene::render() {
    render_system_->update(registry_, context_.getRenderer(), context_.getCamera());

    Scene::render();
}

void GameScene::clean() {

    Scene::clean();
}

void GameScene::testResourceManager() {
    // 载入资源
    context_.getResourceManager().loadTexture("assets/textures/Buildings/Castle.png"_hs);
    // 播放音乐
    context_.getAudioPlayer().playMusic("battle_bgm"_hs);
    
    // 测试UI元素（使用载入的资源）
    ui_manager_->addElement(std::make_unique<engine::ui::UIImage>("assets/textures/Buildings/Castle.png"_hs));
    ui_manager_->addElement(std::make_unique<engine::ui::UILabel>(
        context_.getTextRenderer(), 
        "Hello, World!", 
        "assets/fonts/VonwaonBitmap-16px.ttf"
    ));
}

void GameScene::testECS() {
    auto entity = registry_.create();
    // 变换、速度、精灵组件
    registry_.emplace<engine::component::TransformComponent>(entity, glm::vec2(100, 100));
    registry_.emplace<engine::component::VelocityComponent>(entity, glm::vec2(10, 10));
    registry_.emplace<engine::component::SpriteComponent>(entity, 
        engine::component::Sprite("assets/textures/Units/Archer.png", engine::utils::Rect(0, 0, 192, 192)));
    
    // 动画组件 (单一动画 -> 动画map -> AnimationComponent)
    auto animation = engine::component::Animation(
        {
            engine::component::AnimationFrame(engine::utils::Rect(0, 0, 192, 192), 100),
            engine::component::AnimationFrame(engine::utils::Rect(192, 0, 192, 192), 100),
            engine::component::AnimationFrame(engine::utils::Rect(384, 0, 192, 192), 100),
            engine::component::AnimationFrame(engine::utils::Rect(576, 0, 192, 192), 100),
            engine::component::AnimationFrame(engine::utils::Rect(768, 0, 192, 192), 100),
            engine::component::AnimationFrame(engine::utils::Rect(960, 0, 192, 192), 100)
        }
    );
    auto animation_map = std::unordered_map<entt::id_type, engine::component::Animation>{{"idle"_hs, std::move(animation)}};
    registry_.emplace<engine::component::AnimationComponent>(entity, std::move(animation_map), "idle"_hs);
}

} // namespace game::scene
