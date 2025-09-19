#include "game_scene.h"
#include "../component/player_component.h"
#include "../component/stats_component.h"
#include "../factory/entity_factory.h"
#include "../factory/blueprint_manager.h"
#include "../loader/entity_builder_mw.h"
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
#include "../defs/tags.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/render/camera.h"
#include "../../engine/system/render_system.h"
#include "../../engine/system/movement_system.h"
#include "../../engine/system/animation_system.h"
#include "../../engine/system/ysort_system.h"
#include "../../engine/system/audio_system.h"
#include "../../engine/loader/level_loader.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/ui/ui_label.h"
#include <entt/core/hashed_string.hpp>
#include <entt/signal/sigh.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::scene {

GameScene::GameScene(engine::core::Context& context)
    : engine::scene::Scene("GameScene", context) {
    spdlog::info("GameScene 构造完成");
}

GameScene::~GameScene() {
}

void GameScene::init() {
    if (!initSessionData()) {
        spdlog::error("初始化session_data_失败");
        return;
    }
    if (!initUIConfig()) {
        spdlog::error("初始化UI配置失败");
        return;
    }
    if (!loadLevel()) {
        spdlog::error("加载关卡失败");
        return;
    }
    if (!initEventConnections()) {
        spdlog::error("初始化事件连接失败");
        return;
    }
    if (!initInputConnections()) {
        spdlog::error("初始化输入连接失败");
        return;
    }
    if (!initEntityFactory()) {
        spdlog::error("初始化实体工厂失败");
        return;
    }
    if (!initSystems()) {
        spdlog::error("初始化系统失败");
        return;
    }

    testSessionData();
    createTestEnemy();
    createUnitsPortraitUI();
    Scene::init();
}

void GameScene::update(float delta_time) {
    auto& dispatcher = context_.getDispatcher();

    // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
    remove_dead_system_->update(registry_);

    // 注意系统更新的顺序
    timer_system_->update(registry_, delta_time);
    block_system_->update(registry_, dispatcher);
    set_target_system_->update(registry_);
    follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
    orientation_system_->update(registry_);     // 调用顺序要在Block、SetTarget、FollowPath之后
    attack_starter_system_->update(registry_, dispatcher);
    projectile_system_->update(delta_time);
    movement_system_->update(registry_, delta_time);
    animation_system_->update(delta_time);
    ysort_system_->update(registry_);   // 调用顺序要在MovementSystem之后
    Scene::update(delta_time);
}

void GameScene::render() {
    auto& renderer = context_.getRenderer();
    auto& camera = context_.getCamera();
    
    // 注意渲染顺序，保证正确的遮盖关系
    render_system_->update(registry_, renderer, camera);
    health_bar_system_->update(registry_, renderer, camera);

    Scene::render();
}

void GameScene::clean() {
    auto& dispatcher = context_.getDispatcher();
    auto& input_manager = context_.getInputManager();
    // 断开所有事件连接
    dispatcher.disconnect(this);
    // 断开输入信号连接
    input_manager.onAction("mouse_right"_hs).disconnect<&GameScene::onCreateTestPlayerMelee>(this);
    input_manager.onAction("mouse_left"_hs).disconnect<&GameScene::onCreateTestPlayerRanged>(this);
    input_manager.onAction("pause"_hs).disconnect<&GameScene::onClearAllPlayers>(this);
    input_manager.onAction("move_left"_hs).disconnect<&GameScene::onCreateTestPlayerHealer>(this);
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
    if (!level_loader.loadLevel("assets/maps/level1.tmj", this)) {
        spdlog::error("加载关卡失败");
        return false;
    }
    return true;
}

bool GameScene::initEventConnections() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<game::defs::EnemyArriveHomeEvent>().connect<&GameScene::onEnemyArriveHome>(this);
    return true;
}

bool GameScene::initInputConnections() {
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).connect<&GameScene::onCreateTestPlayerMelee>(this);
    input_manager.onAction("mouse_left"_hs).connect<&GameScene::onCreateTestPlayerRanged>(this);
    input_manager.onAction("pause"_hs).connect<&GameScene::onClearAllPlayers>(this);
    input_manager.onAction("move_left"_hs).connect<&GameScene::onCreateTestPlayerHealer>(this);
    return true;
}

bool GameScene::initEntityFactory() {
    // 如果蓝图管理器为空，则创建一个（将来可能由构造函数传入）
    if (!blueprint_manager_) {  
        blueprint_manager_ = std::make_shared<game::factory::BlueprintManager>(context_.getResourceManager());
        if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
            !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
            !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json")) {
            spdlog::error("加载蓝图失败");
            return false;
        }
    }
    entity_factory_ = std::make_unique<game::factory::EntityFactory>(registry_, *blueprint_manager_);
    spdlog::info("entity_factory_ 加载完成");
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
    timer_system_ = std::make_unique<game::system::TimerSystem>();
    orientation_system_ = std::make_unique<game::system::OrientationSystem>();
    animation_state_system_ = std::make_unique<game::system::AnimationStateSystem>(registry_, dispatcher);
    animation_event_system_ = std::make_unique<game::system::AnimationEventSystem>(registry_, dispatcher);
    combat_resolve_system_ = std::make_unique<game::system::CombatResolveSystem>(registry_, dispatcher);
    projectile_system_ = std::make_unique<game::system::ProjectileSystem>(registry_, dispatcher, *entity_factory_);
    effect_system_ = std::make_unique<game::system::EffectSystem>(registry_, dispatcher, *entity_factory_);
    health_bar_system_ = std::make_unique<game::system::HealthBarSystem>();
    spdlog::info("系统初始化完成");
    return true;
}

void GameScene::createUnitsPortraitUI() {
    if (!ui_manager_->init(context_.getGameState().getLogicalSize())) return;

    auto padding = ui_config_->getUnitPanelPadding();
    auto& unit_map = session_data_->getUnitMap();
    auto unit_num = unit_map.size();

    // --- 在屏幕下方创建一个panel UI 条，用于显示角色肖像 ---
    // 获取窗口大小和角色肖像框大小
    auto window_size = context_.getGameState().getLogicalSize();
    auto frame_size = ui_config_->getUnitPanelFrameSize();
    // 根据角色数量、角色肖像框大小、间隔计算panel的位置和大小
    auto pos = glm::vec2(0.0f, window_size.y - frame_size.y - 2 * padding);
    auto size = glm::vec2(unit_num*frame_size.x + (unit_num + 1)*padding, frame_size.y + 2 * padding);
    auto anchor_panel = std::make_unique<engine::ui::UIPanel>(pos, size);
    // 设置背景色
    anchor_panel->setBackgroundColor(engine::utils::FColor(0.1f, 0.1f, 0.1f, 0.1f));
    // 设置ID，以后即可根据ID找到该panel
    anchor_panel->setId("unit_panel"_hs);

    // 依次添加角色肖像，每个肖像显示由四部分依次叠加：portrait，frame，icon，cost，可以通过一个frame_panel定位（位于上层anchor_panel之中）
    int index = 0;
    for (auto& [name_id, unit_data] : unit_map) {
        auto portrait = ui_config_->getPortrait(name_id);
        auto frame = ui_config_->getPortraitFrame(unit_data.rarity_);
        auto icon = ui_config_->getIcon(unit_data.class_id_);
        auto cost = blueprint_manager_->getPlayerClassBlueprint(unit_data.class_id_).player_.cost_;
        cost = static_cast<int>(std::round(engine::utils::statModify(cost, 1, unit_data.rarity_))); // 只有稀有度对cost有影响

        // 创建每个肖像的 frame_panel
        auto frame_pos = glm::vec2(padding + index * (frame_size.x + padding), padding);
        auto frame_panel = std::make_unique<engine::ui::UIPanel>(frame_pos, frame_size);
        frame_panel->setId(name_id);

        // 依次添加四个元素，为了能够交互，将frame设置为按钮，并绑定点击事件
        frame_panel->addChild(std::make_unique<engine::ui::UIImage>(portrait, glm::vec2(0.0f, 0.0f), frame_size));
        frame_panel->addChild(std::make_unique<engine::ui::UIButton>(context_, 
            frame, 
            frame, 
            frame, 
            glm::vec2(0.0f, 0.0f), 
            frame_size
            // TODO: 添加点击事件回调函数
        ));
        frame_panel->addChild(std::make_unique<engine::ui::UIImage>(icon, glm::vec2(0.0f, 0.0f), frame_size / 2.0f));
        frame_panel->addChild(std::make_unique<engine::ui::UILabel>(context_.getTextRenderer(), 
            std::to_string(cost), 
            ui_config_->getUnitPanelFontPath(), 
            ui_config_->getUnitPanelFontSize(), 
            engine::utils::FColor::yellow(), 
            ui_config_->getUnitPanelFontOffset()
        ));
        // 最后添加一个灰色的遮盖panel，cost不足以支持该角色出击时显示
        auto cover_panel = std::make_unique<engine::ui::UIPanel>(glm::vec2(0.0f, 0.0f), frame_size);
        cover_panel->setBackgroundColor(engine::utils::FColor(0.0f, 0.0f, 0.0f, 0.2f));
        cover_panel->setId("cover_panel"_hs);
        frame_panel->addChild(std::move(cover_panel));

        // 将frame_panel添加到anchor_panel中，并使用cost作为排序键
        anchor_panel->addChild(std::move(frame_panel), cost);
        index++;
    }
    
    // 对anchor_panel中的子元素(frame_panel)进行排序
    anchor_panel->sortChildrenByOrderIndex();
    // 按顺序排列anchor_panel中的子元素(frame_panel)的位置
    arrangeUnitsPortraitUI(anchor_panel.get(), frame_size, padding);

    ui_manager_->addElement(std::move(anchor_panel));
}

void GameScene::arrangeUnitsPortraitUI(engine::ui::UIElement* anchor_panel, const glm::vec2& frame_size, float padding) {
    // 遍历panel中的子元素(定位panel)，并依次设定位置
    for (size_t i = 0; i < anchor_panel->getChildren().size(); i++) {
        auto& child = anchor_panel->getChildren()[i];
        child->setPosition(glm::vec2(padding + i * (frame_size.x + padding), padding));
    }
    // 更新panel的size
    anchor_panel->setSize(glm::vec2(padding + anchor_panel->getChildren().size() * (frame_size.x + padding), 
                                    frame_size.y + 2 * padding));
}

// --- 事件回调函数 ---
void GameScene::onEnemyArriveHome(const game::defs::EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    // TODO: 添加敌人到达基地的逻辑
}

// --- 测试函数 ---
void GameScene::testSessionData() {
    spdlog::info("关卡号: {}", level_number_);
    spdlog::info("积分: {}", session_data_->getPoint());
    spdlog::info("是否通关: {}", session_data_->isLevelClear());
    for (auto& unit : session_data_->getUnitMap()) {
        spdlog::info("角色名: {}, 职业: {}, 等级: {}, 稀有度: {}", unit.second.name_, unit.second.class_, unit.second.level_, unit.second.rarity_);
    }
}

void GameScene::createTestEnemy() {
    // 每个起点创建一批敌人
    for (auto start_index : start_points_) {
        auto position = waypoint_nodes_[start_index].position_;

        entity_factory_->createEnemyUnit("wolf"_hs, position, start_index);
        entity_factory_->createEnemyUnit("slime"_hs, position, start_index);
        entity_factory_->createEnemyUnit("goblin"_hs, position, start_index);
        entity_factory_->createEnemyUnit("dark_witch"_hs, position, start_index);
    }
}

bool GameScene::onCreateTestPlayerMelee() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    auto entity = entity_factory_->createPlayerUnit("warrior"_hs, position);
    // 让玩家处于受伤状态（治疗师不会锁定满血目标）
    registry_.emplace<game::defs::InjuredTag>(entity);
    auto& stats = registry_.get<game::component::StatsComponent>(entity);
    stats.hp_ = stats.max_hp_ / 2;
    spdlog::info("创建战士: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onCreateTestPlayerRanged() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    auto entity = entity_factory_->createPlayerUnit("archer"_hs, position);
    // 让玩家处于受伤状态（治疗师不会锁定满血目标）
    registry_.emplace<game::defs::InjuredTag>(entity);
    auto& stats = registry_.get<game::component::StatsComponent>(entity);
    stats.hp_ = stats.max_hp_ / 2;
    spdlog::info("创建弓箭手: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onCreateTestPlayerHealer() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    entity_factory_->createPlayerUnit("witch"_hs, position);
    spdlog::info("创建治疗者: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onClearAllPlayers() {
    auto view = registry_.view<game::component::PlayerComponent>();
    for (auto entity : view) {
        registry_.destroy(entity);
    }
    return true;
}



} // namespace game::scene
