#include "game_scene.h"
#include "../../engine/core/context.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/resource/resource_manager.h"
#include "../../engine/render/text_renderer.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_label.h"
#include <entt/core/hashed_string.hpp>
#include <entt/signal/sigh.hpp>

using namespace entt::literals;

namespace game::scene {

GameScene::GameScene(engine::core::Context& context)
    : engine::scene::Scene("GameScene", context) {
}

GameScene::~GameScene() {
}

void GameScene::init() {
    // 测试资源管理器
    testResourceManager();

    Scene::init();
}

void GameScene::clean() {

    Scene::clean();
}

void GameScene::testResourceManager() {
    // 载入资源
    context_.getResourceManager().loadTexture("assets/textures/Buildings/Castle.png"_hs);
    // 播放音乐
    context_.getAudioPlayer().playMusic("assets/audio/4 Battle Track INTRO TomMusic.ogg"_hs);
    
    // 测试UI元素（使用载入的资源）
    ui_manager_->addElement(std::make_unique<engine::ui::UIImage>("assets/textures/Buildings/Castle.png"_hs));
    ui_manager_->addElement(std::make_unique<engine::ui::UILabel>(
        context_.getTextRenderer(), 
        "Hello, World!", 
        "assets/fonts/VonwaonBitmap-16px.ttf"
    ));
}

} // namespace game::scene
