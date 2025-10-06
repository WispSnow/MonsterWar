#pragma once
#include "../../engine/scene/scene.h"
#include "../../engine/system/fwd.h"
#include "../../game/data/ui_config.h"
#include "../../game/data/session_data.h"
#include "../../game/data/level_config.h"
#include "../../game/factory/blueprint_manager.h"
#include "../system/fwd.h"

namespace game::scene {

class TitleScene final: public engine::scene::Scene {
    friend class game::system::DebugUISystem;   // 允许DebugUISystem访问私有成员变量及方法

    // 数据相关实例
    std::shared_ptr<game::factory::BlueprintManager> blueprint_manager_;
    std::shared_ptr<game::data::SessionData> session_data_;
    std::shared_ptr<game::data::UIConfig> ui_config_;
    std::shared_ptr<game::data::LevelConfig> level_config_;

    // 系统相关实例
    std::unique_ptr<engine::system::RenderSystem> render_system_;
    std::unique_ptr<engine::system::YSortSystem> ysort_system_;
    std::unique_ptr<engine::system::AnimationSystem> animation_system_;
    std::unique_ptr<engine::system::MovementSystem> movement_system_;
    std::unique_ptr<game::system::DebugUISystem> debug_ui_system_;

    bool show_unit_info_{false};        ///< @brief 是否显示角色列表UI
    bool show_load_panel_{false};       ///< @brief 是否显示加载面板UI

public:
    TitleScene(engine::core::Context& context,
        std::shared_ptr<game::factory::BlueprintManager> blueprint_manager = nullptr,
        std::shared_ptr<game::data::SessionData> session_data = nullptr,
        std::shared_ptr<game::data::UIConfig> ui_config = nullptr,
        std::shared_ptr<game::data::LevelConfig> level_config = nullptr);
    ~TitleScene();

    bool init() override;
    void update(float delta_time) override;
    void render() override;

private:
    // 初始化函数(init函数中调用)
    [[nodiscard]] bool initSessionData();
    [[nodiscard]] bool initLevelConfig();
    [[nodiscard]] bool initBlueprintManager();
    [[nodiscard]] bool initUIConfig();
    [[nodiscard]] bool loadTitleLevel();
    [[nodiscard]] bool initSystems();
    [[nodiscard]] bool initRegistryContext();
    [[nodiscard]] bool initUI();

    // 按钮回调函数 (未来通过游戏UI调用)
    void onStartGameClick();
    void onConfirmRoleClick();
    void onLoadGameClick();
    void onQuitClick();
};

}