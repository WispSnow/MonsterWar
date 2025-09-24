#pragma once
#include "../data/waypoint_node.h"
#include "../data/session_data.h"
#include "../data/ui_config.h"
#include "../data/game_stats.h"
#include "../data/level_config.h"
#include "../system/fwd.h"
#include "../../engine/scene/scene.h"
#include "../../engine/system/fwd.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace engine::ui {
    class UIElement;
}

namespace game::ui {
    class UnitsPortraitUI;
}

namespace game::factory {
    class EntityFactory;
    class BlueprintManager;
}

namespace game::spawner {
    class EnemySpawner;
}

namespace game::scene {

class GameScene final: public engine::scene::Scene {
private:
    std::unique_ptr<engine::system::RenderSystem> render_system_;
    std::unique_ptr<engine::system::MovementSystem> movement_system_;
    std::unique_ptr<engine::system::AnimationSystem> animation_system_;
    std::unique_ptr<engine::system::YSortSystem> ysort_system_;
    std::unique_ptr<engine::system::AudioSystem> audio_system_;

    std::unique_ptr<game::system::FollowPathSystem> follow_path_system_;
    std::unique_ptr<game::system::RemoveDeadSystem> remove_dead_system_;
    std::unique_ptr<game::system::BlockSystem> block_system_;
    std::unique_ptr<game::system::SetTargetSystem> set_target_system_;
    std::unique_ptr<game::system::AttackStarterSystem> attack_starter_system_;
    std::unique_ptr<game::system::TimerSystem> timer_system_;
    std::unique_ptr<game::system::OrientationSystem> orientation_system_;
    std::unique_ptr<game::system::AnimationStateSystem> animation_state_system_;
    std::unique_ptr<game::system::AnimationEventSystem> animation_event_system_;
    std::unique_ptr<game::system::CombatResolveSystem> combat_resolve_system_;
    std::unique_ptr<game::system::ProjectileSystem> projectile_system_;
    std::unique_ptr<game::system::EffectSystem> effect_system_;
    std::unique_ptr<game::system::HealthBarSystem> health_bar_system_;
    std::unique_ptr<game::system::GameRuleSystem> game_rule_system_;
    std::unique_ptr<game::system::PlaceUnitSystem> place_unit_system_;
    std::unique_ptr<game::system::RenderRangeSystem> render_range_system_;
    
    std::unique_ptr<game::spawner::EnemySpawner> enemy_spawner_;        // 敌人生成器，负责生成敌人
    std::unique_ptr<game::ui::UnitsPortraitUI> units_portrait_ui_;      // 封装的单位肖像UI，负责管理单位肖像UI的创建、更新和排列

    std::unordered_map<int, game::data::WaypointNode> waypoint_nodes_;  // 路径节点ID到节点数据的映射
    std::vector<int> start_points_;                                     // 起点ID列表
    game::data::GameStats game_stats_;                                  // 关卡内游戏统计数据
    game::data::Waves waves_;                                           // 关卡波次数据

    std::unique_ptr<game::factory::EntityFactory> entity_factory_;      // 实体工厂，负责创建和管理实体

    // 管理数据的实例很可能同时被多个场景使用，因此使用共享指针
    std::shared_ptr<game::factory::BlueprintManager> blueprint_manager_;// 蓝图管理器，负责管理蓝图数据
    std::shared_ptr<game::data::SessionData> session_data_;             // 会话数据，关卡切换时需要传递的数据
    std::shared_ptr<game::data::UIConfig> ui_config_;                   // UI配置，负责管理UI数据
    std::shared_ptr<game::data::LevelConfig> level_config_;             // 关卡配置，负责管理关卡数据

    // --- 其他场景数据 ---
    int level_number_{1};
    
public:
    GameScene(engine::core::Context& context);
    ~GameScene();

    void init() override;
    void update(float delta_time) override;
    void render() override;
    void clean() override;

private:
    [[nodiscard]] bool initSessionData();
    [[nodiscard]] bool initLevelConfig();
    [[nodiscard]] bool initUIConfig();
    [[nodiscard]] bool loadLevel();
    [[nodiscard]] bool initEventConnections();
    [[nodiscard]] bool initInputConnections();
    [[nodiscard]] bool initEntityFactory();
    [[nodiscard]] bool initRegistryContext();
    [[nodiscard]] bool initSystems();
    [[nodiscard]] bool initEnemySpawner();
    [[nodiscard]] bool initUnitsPortraitUI();

    // 测试函数
    bool onClearAllPlayers();

};

} // namespace game::scene