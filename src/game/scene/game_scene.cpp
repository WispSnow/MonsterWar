#include "game_scene.h"
#include "title_scene.h"
#include "level_clear_scene.h"
#include "end_scene.h"
#include "../factory/entity_factory.h"
#include "../factory/blueprint_manager.h"
#include "../loader/entity_builder_mw.h"
#include "../spawner/enemy_spawner.h"
#include "../system/followpath_system.h"
#include "../system/remove_dead_system.h"
#include "../system/block_system.h"
#include "../system/set_target_system.h"
#include "../system/attack_starter_system.h"
#include "../system/timer_system.h"
#include "../system/orientation_system.h"
#include "../system/animation_state_system.h"
#include "../system/animation_event_system.h"
#include "../system/combat_resolve_system.h"
#include "../system/projectile_system.h"
#include "../system/effect_system.h"
#include "../system/health_bar_system.h"
#include "../system/game_rule_system.h"
#include "../system/place_unit_system.h"
#include "../system/render_range_system.h"
#include "../system/debug_ui_system.h"
#include "../system/selection_system.h"
#include "../system/skill_system.h"
#include "../ui/units_portrait_ui.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/system/audio_system.h"
#include "../../engine/loader/level_loader.h"
#include "../../engine/ui/ui_manager.h"
#include <entt/core/hashed_string.hpp>
#include <entt/signal/sigh.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::scene {

GameScene::GameScene(engine::core::Context& context,
    std::shared_ptr<game::factory::BlueprintManager> blueprint_manager,
    std::shared_ptr<game::data::SessionData> session_data,
    std::shared_ptr<game::data::UIConfig> ui_config,
    std::shared_ptr<game::data::LevelConfig> level_config)
    : engine::scene::Scene("GameScene", context),
      blueprint_manager_(blueprint_manager),
      session_data_(session_data),
      ui_config_(ui_config),
      level_config_(level_config)
{
    spdlog::info("GameScene 构造完成");
}

GameScene::~GameScene() {
}

void GameScene::init() {
    if (!initSessionData())         { spdlog::error("初始化session_data_失败"); return; }
    if (!initLevelConfig())         { spdlog::error("初始化关卡配置失败"); return; }
    if (!initUIConfig())            { spdlog::error("初始化UI配置失败"); return; }
    if (!loadLevel())               { spdlog::error("加载关卡失败"); return; }
    if (!initEventConnections())    { spdlog::error("初始化事件连接失败"); return; }
    if (!initInputConnections())    { spdlog::error("初始化输入连接失败"); return; }
    if (!initEntityFactory())       { spdlog::error("初始化实体工厂失败"); return; }
    if (!initRegistryContext())     { spdlog::error("初始化注册表上下文失败"); return; }
    if (!initUnitsPortraitUI())     { spdlog::error("初始化单位肖像UI失败"); return; }
    if (!initSystems())             { spdlog::error("初始化系统失败"); return; }
    if (!initEnemySpawner())        { spdlog::error("初始化敌人生成器失败"); return; }

    context_.getGameState().setState(engine::core::State::Playing);
    context_.getAudioPlayer().playMusic("battle_bgm"_hs);
    Scene::init();
}

void GameScene::update(float delta_time) {
    auto& dispatcher = context_.getDispatcher();

    // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
    remove_dead_system_->update(registry_);

    // 暂停状态下，有些功能依然正常运行
    if (context_.getGameState().isPaused()) {
        place_unit_system_->update(delta_time);
        ysort_system_->update(registry_);
        selection_system_->update();
        units_portrait_ui_->update(delta_time);
        Scene::update(delta_time);
        return;
    }

    // 注意系统更新的顺序
    timer_system_->update(delta_time);
    game_rule_system_->update(delta_time);
    block_system_->update(registry_, dispatcher);
    set_target_system_->update(registry_);
    follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
    orientation_system_->update(registry_);     // 调用顺序要在Block、SetTarget、FollowPath之后
    attack_starter_system_->update(registry_, dispatcher);
    projectile_system_->update(delta_time);
    movement_system_->update(registry_, delta_time);
    animation_system_->update(delta_time);
    place_unit_system_->update(delta_time);
    ysort_system_->update(registry_);   // 调用顺序要在MovementSystem之后
    selection_system_->update();

    // 场景中其他更新函数
    enemy_spawner_->update(delta_time);
    units_portrait_ui_->update(delta_time);
    Scene::update(delta_time);
}

void GameScene::render() {
    auto& renderer = context_.getRenderer();
    auto& camera = context_.getCamera();
    
    // 注意渲染顺序，保证正确的遮盖关系
    render_system_->update(registry_, renderer, camera);
    health_bar_system_->update(registry_, renderer, camera);
    render_range_system_->update(registry_, renderer, camera);

    Scene::render();
    // 当场景栈中只有GameScene时才渲染调试UI, 不然上层有其它场景时会冲突
    if (context_.getGameState().isPlaying() || context_.getGameState().isPaused()) {
        debug_ui_system_->update();     // 调试UI的显示优先级最高，最后渲染
    }
}

void GameScene::clean() {
    auto& dispatcher = context_.getDispatcher();
    // 断开所有事件连接
    dispatcher.disconnect(this);
    // 断开输入信号连接
    Scene::clean();
}

bool GameScene::initSessionData() {
    if (!session_data_) {
        session_data_ = std::make_shared<game::data::SessionData>();
        if (!session_data_->loadDefaultData()) {
            spdlog::error("初始化session_data_失败");
            return false;
        }
    }
    level_number_ = session_data_->getLevelNumber();
    return true;
}

bool GameScene::initLevelConfig() {
    if (!level_config_) {
        level_config_ = std::make_shared<game::data::LevelConfig>();
        if (!level_config_->loadFromFile("assets/data/level_config.json")) {
            spdlog::error("加载关卡配置失败");
            return false;
        }
    }
    waves_ = level_config_->getWavesData(level_number_);
    game_stats_.enemy_count_ = level_config_->getTotalEnemyCount(level_number_);
    return true;
}

bool GameScene::initUIConfig() {
    if (!ui_config_) {
        ui_config_ = std::make_shared<game::data::UIConfig>();
        if (!ui_config_->loadFromFile("assets/data/ui_config.json")) {
            spdlog::error("加载UI配置失败");
            return false;
        }
    }
    return true;
}

bool GameScene::loadLevel() {
    engine::loader::LevelLoader level_loader;
    // 设置拓展的构建器EntityBuilderMW
    level_loader.setEntityBuilder(std::make_unique<game::loader::EntityBuilderMW>(level_loader, 
        context_, 
        registry_, 
        waypoint_nodes_, 
        start_points_)
    );
    // 获取关卡地图路径
    auto map_path = level_config_->getMapPath(level_number_);
    if (!level_loader.loadLevel(map_path, this)) {
        spdlog::error("加载关卡失败");
        return false;
    }
    return true;
}

bool GameScene::initEventConnections() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<game::defs::RestartEvent>().connect<&GameScene::onRestart>(this);
    dispatcher.sink<game::defs::BackToTitleEvent>().connect<&GameScene::onBackToTitle>(this);
    dispatcher.sink<game::defs::SaveEvent>().connect<&GameScene::onSave>(this);
    dispatcher.sink<game::defs::LevelClearEvent>().connect<&GameScene::onLevelClear>(this);
    dispatcher.sink<game::defs::GameEndEvent>().connect<&GameScene::onGameEndEvent>(this);
    return true;
}

bool GameScene::initInputConnections() {
    // 未来可添加输入控制，记得在close函数中断开
    return true;
}

bool GameScene::initEntityFactory() {
    // 如果蓝图管理器为空，则创建一个（将来可能由构造函数传入）
    if (!blueprint_manager_) {  
        blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
        if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
            !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
            !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json") ||
            !blueprint_manager_->loadEffectBlueprints("assets/data/effect_data.json") ||
            !blueprint_manager_->loadSkillBlueprints("assets/data/skill_data.json")) {
            spdlog::error("加载蓝图失败");
            return false;
        }
    }
    entity_factory_ = std::make_unique<game::factory::EntityFactory>(registry_, *blueprint_manager_);
    spdlog::info("entity_factory_ 加载完成");
    return true;
}

bool GameScene::initRegistryContext() {
    // 让注册表存储一些数据类型实例作为上下文，方便使用
    registry_.ctx().emplace<std::shared_ptr<game::factory::BlueprintManager>>(blueprint_manager_);
    registry_.ctx().emplace<std::shared_ptr<game::data::SessionData>>(session_data_);
    registry_.ctx().emplace<std::shared_ptr<game::data::UIConfig>>(ui_config_);
    registry_.ctx().emplace<std::shared_ptr<game::data::LevelConfig>>(level_config_);
    registry_.ctx().emplace<std::unordered_map<int, game::data::WaypointNode>&>(waypoint_nodes_);
    registry_.ctx().emplace<std::vector<int>&>(start_points_);
    registry_.ctx().emplace<game::data::GameStats&>(game_stats_);
    registry_.ctx().emplace<game::data::Waves&>(waves_);
    registry_.ctx().emplace<int&>(level_number_);
    registry_.ctx().emplace_as<entt::entity&>("selected_unit"_hs, selected_unit_);
    registry_.ctx().emplace_as<entt::entity&>("hovered_unit"_hs, hovered_unit_);
    registry_.ctx().emplace_as<bool&>("show_save_panel"_hs, show_save_panel_);
    spdlog::info("registry_ 上下文初始化完成");
    return true;
}

bool GameScene::initUnitsPortraitUI() {
    try {
        units_portrait_ui_ = std::make_unique<game::ui::UnitsPortraitUI>(registry_, *ui_manager_, context_);
    } catch (const std::exception& e) {
        spdlog::error("初始化单位肖像UI失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameScene::initSystems() {
    auto& dispatcher = context_.getDispatcher();
    // 系统初始化需要在可能的依赖模块(如实体工厂)初始化之后
    render_system_ = std::make_unique<engine::system::RenderSystem>();
    movement_system_ = std::make_unique<engine::system::MovementSystem>();
    animation_system_ = std::make_unique<engine::system::AnimationSystem>(registry_, dispatcher);
    ysort_system_ = std::make_unique<engine::system::YSortSystem>();
    audio_system_ = std::make_unique<engine::system::AudioSystem>(registry_, context_);

    follow_path_system_ = std::make_unique<game::system::FollowPathSystem>();
    remove_dead_system_ = std::make_unique<game::system::RemoveDeadSystem>();
    block_system_ = std::make_unique<game::system::BlockSystem>();
    set_target_system_ = std::make_unique<game::system::SetTargetSystem>();
    attack_starter_system_ = std::make_unique<game::system::AttackStarterSystem>();
    timer_system_ = std::make_unique<game::system::TimerSystem>(registry_, dispatcher);
    orientation_system_ = std::make_unique<game::system::OrientationSystem>();
    animation_state_system_ = std::make_unique<game::system::AnimationStateSystem>(registry_, dispatcher);
    animation_event_system_ = std::make_unique<game::system::AnimationEventSystem>(registry_, dispatcher);
    combat_resolve_system_ = std::make_unique<game::system::CombatResolveSystem>(registry_, dispatcher);
    projectile_system_ = std::make_unique<game::system::ProjectileSystem>(registry_, dispatcher, *entity_factory_);
    effect_system_ = std::make_unique<game::system::EffectSystem>(registry_, dispatcher, *entity_factory_);
    health_bar_system_ = std::make_unique<game::system::HealthBarSystem>();
    game_rule_system_ = std::make_unique<game::system::GameRuleSystem>(registry_, dispatcher);
    place_unit_system_ = std::make_unique<game::system::PlaceUnitSystem>(registry_, *entity_factory_, context_);
    render_range_system_ = std::make_unique<game::system::RenderRangeSystem>();
    debug_ui_system_ = std::make_unique<game::system::DebugUISystem>(registry_, context_);
    selection_system_ = std::make_unique<game::system::SelectionSystem>(registry_, context_);
    skill_system_ = std::make_unique<game::system::SkillSystem>(registry_, dispatcher, *entity_factory_);
    spdlog::info("系统初始化完成");
    return true;
}

bool GameScene::initEnemySpawner() {
    enemy_spawner_ = std::make_unique<game::spawner::EnemySpawner>(registry_, *entity_factory_);
    spdlog::info("敌人生成器初始化完成");
    return true;
}

// --- 场景相关函数 ---
void GameScene::onRestart() {
    spdlog::info("重新开始关卡");
    requestReplaceScene(std::make_unique<game::scene::GameScene>(
        context_, 
        blueprint_manager_,
        session_data_,
        ui_config_,
        level_config_
        )
    );
}

void GameScene::onBackToTitle() {
    spdlog::info("返回标题");
    requestReplaceScene(std::make_unique<game::scene::TitleScene>(context_));
}

void GameScene::onSave() {
    spdlog::info("保存");
    show_save_panel_ = !show_save_panel_;
    /* 用ImGui快速实现逻辑，未来再完善游戏内UI */
}

void GameScene::onLevelClear() {
    spdlog::info("关卡通关成功");
    // 奖励点数 = 击杀数 + 基地血量 * 5
    const auto point = game_stats_.enemy_killed_count_ + game_stats_.home_hp_ * 5;
    session_data_->setLevelClear(true);
    session_data_->addPoint(point);

    // 如果当前关卡是最后一关，则进入结束场景；否则进入通关结算场景
    if (level_config_->isFinalLevel(level_number_)) {
        requestPushScene(std::make_unique<game::scene::EndScene>(context_, true));
    } else {
        requestPushScene(std::make_unique<game::scene::LevelClearScene>(
        context_,
        blueprint_manager_,
        ui_config_,
        level_config_,
        session_data_,
        game_stats_
        ));
    }
}

void GameScene::onGameEndEvent(const game::defs::GameEndEvent& event) {
    spdlog::info("游戏结束");
    requestPushScene(std::make_unique<game::scene::EndScene>(context_, event.is_win_));
}

} // namespace game::scene
