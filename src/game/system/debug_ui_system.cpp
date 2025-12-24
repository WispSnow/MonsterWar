#include "debug_ui_system.h"
#include "../component/stats_component.h"
#include "../component/class_name_component.h"
#include "../component/blocker_component.h"
#include "../component/skill_component.h"
#include "../component/player_component.h"
#include "../defs/tags.h"
#include "../defs/events.h"
#include "../data/game_stats.h"
#include "../data/level_data.h"
#include "../data/session_data.h"
#include "../factory/blueprint_manager.h"
#include "../scene/title_scene.h"
#include "../scene/level_clear_scene.h"
#include "../scene/end_scene.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/component/name_component.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/core/time.h"
#include "../../engine/render/renderer.h"
#include "../../engine/resource/resource_manager.h"
#include "../../engine/utils/math.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <entt/core/hashed_string.hpp>

using namespace entt::literals;

namespace game::system {

DebugUISystem::DebugUISystem(entt::registry& registry, engine::core::Context& context)
    : registry_(registry), context_(context) {
    context_.getDispatcher().sink<game::defs::UIPortraitHoverEnterEvent>().connect<&DebugUISystem::onUIPortraitHoverEnterEvent>(this);
    context_.getDispatcher().sink<game::defs::UIPortraitHoverLeaveEvent>().connect<&DebugUISystem::onUIPortraitHoverLeaveEvent>(this);
}

DebugUISystem::~DebugUISystem() {
    context_.getDispatcher().disconnect(this);
}

void DebugUISystem::update() {
    beginFrame();
    renderHoveredPortrait();
    renderHoveredUnit();
    renderSelectedUnit();
    renderInfoUI();
    renderSettingUI();
    renderDebugUI();
    // 渲染可能激活的保存面板
    auto& show_save_panel = registry_.ctx().get<bool&>("show_save_panel"_hs);
    renderSavePanelUI(show_save_panel);
    endFrame();
}

void DebugUISystem::updateTitle(game::scene::TitleScene& title_scene) {
    beginFrame();
    renderTitleLogo();
    renderTitleButtons(title_scene);
    // 渲染可能激活的角色信息和载入面板
    renderUnitInfoUI(title_scene.show_unit_info_);      // 可以直接获取TitleScene的私有成员变量
    renderLoadPanelUI(title_scene.show_load_panel_);
    endFrame();
}

void DebugUISystem::updateLevelClear(game::scene::LevelClearScene& level_clear_scene) {
    beginFrame();
    renderLevelClearText();
    renderLevelClearTable(level_clear_scene);
    renderLevelClearButtons(level_clear_scene);
    renderSavePanelUI(level_clear_scene.show_save_panel_);
    endFrame();
}

void DebugUISystem::updateEnd(game::scene::EndScene& end_scene) {
    beginFrame();
    renderEndText(end_scene);
    renderEndButtons(end_scene);
    endFrame();
}

void DebugUISystem::beginFrame() {
    // 开始新帧
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // 关闭逻辑分辨率 (ImGui目前对于SDL逻辑分辨率支持不好，所以使用时先关闭)
    if (!context_.getGameState().disableLogicalPresentation()) {
        spdlog::error("关闭逻辑分辨率失败");
    }
}

void DebugUISystem::endFrame() {
    // ImGui: 渲染
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), context_.getRenderer().getSDLRenderer());
    
    // 渲染完成后，打开(恢复)逻辑分辨率
    if (!context_.getGameState().enableLogicalPresentation()) {
        spdlog::error("启用逻辑分辨率失败");
    }
}

// ----------------------------- GameScene -----------------------------
void DebugUISystem::renderHoveredPortrait() {
    // 确定鼠标悬浮的单位肖像存在
    if (hovered_portrait_ == entt::null) return;

    // 角色名称不是一个实体，需要从蓝图中获取数据
    const auto& session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    const auto& blueprint_mgr = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    const auto& unit_data = session_data->getUnitData(hovered_portrait_);
    const auto& class_blueprint = blueprint_mgr->getPlayerClassBlueprint(unit_data.class_id_);
    const auto& stats = class_blueprint.stats_;
    // 计算等级和稀有度对属性的影响
    const auto hp = engine::utils::statModify(stats.hp_, unit_data.level_, unit_data.rarity_);
    const auto atk = engine::utils::statModify(stats.atk_, unit_data.level_, unit_data.rarity_);
    const auto def = engine::utils::statModify(stats.def_, unit_data.level_, unit_data.rarity_);
    const auto range = stats.range_;
    std::string_view name = class_blueprint.display_info_.name_;
    // std::string_view description = class_blueprint.display_info_.description_;

    // Display Tooltip info
    if (!ImGui::BeginTooltip()) {
        ImGui::EndTooltip();
        spdlog::error("Hovered unit portrait window failed to open");
        return;
    }
    ImGui::Text("%s", unit_data.name_.c_str());
    ImGui::SameLine();
    ImGui::Text("Class: %s", name.data());
    ImGui::Text("Level: %d", unit_data.level_);
    ImGui::SameLine();
    ImGui::Text("Rarity: %d", unit_data.rarity_);
    ImGui::Text("HP: %d", static_cast<int>(std::round(hp)));
    ImGui::SameLine();
    ImGui::Text("ATK: %d", static_cast<int>(std::round(atk)));
    ImGui::Text("DEF: %d", static_cast<int>(std::round(def)));
    ImGui::SameLine();
    ImGui::Text("Range: %d", static_cast<int>(std::round(range)));
    ImGui::EndTooltip();
}

void DebugUISystem::renderHoveredUnit() {
    // 确定鼠标悬浮的单位存在
    auto& entity = registry_.ctx().get<entt::entity&>("hovered_unit"_hs);
    if (entity == entt::null || !registry_.valid(entity)) return;

    // Tooltip is a small window hovering over the mouse, displaying unit info
    if (!ImGui::BeginTooltip()) {
        ImGui::EndTooltip();
        spdlog::error("Hovered unit window failed to open");
        return;
    }
    // Get necessary info and display
    const auto& stats = registry_.get<game::component::StatsComponent>(entity);
    const auto& class_name = registry_.get<game::component::ClassNameComponent>(entity);
    // Only player units have names, so try to get it
    if (auto name = registry_.try_get<engine::component::NameComponent>(entity); name) {
        ImGui::Text("%s  ", name->name_.c_str());
        ImGui::SameLine();
    }
    ImGui::Text("%s", class_name.class_name_.c_str());
    ImGui::Text("Level: %d", stats.level_);
    ImGui::SameLine();
    ImGui::Text("Rarity: %d", stats.rarity_);
    ImGui::Text("HP: %d/%d", static_cast<int>(std::round(stats.hp_)), static_cast<int>(std::round(stats.max_hp_)));
    ImGui::Text("ATK: %d", static_cast<int>(std::round(stats.atk_)));
    ImGui::Text("DEF: %d", static_cast<int>(std::round(stats.def_)));
    ImGui::Text("Range: %d", static_cast<int>(std::round(stats.range_)));
    ImGui::Text("Interval: %.2f", stats.atk_interval_);
    ImGui::EndTooltip();
}

void DebugUISystem::renderSelectedUnit() {
    // 确定选中的单位存在
    auto& entity = registry_.ctx().get<entt::entity&>("selected_unit"_hs);
    if (entity == entt::null || !registry_.valid(entity)) return;

    // Set window position top-left
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);

    if (!ImGui::Begin("Unit Status", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("Unit Status window failed to open");
        return;
    }
    // Get necessary info and display
    const auto& stats = registry_.get<game::component::StatsComponent>(entity);
    const auto& class_name = registry_.get<game::component::ClassNameComponent>(entity);
    const auto blocker = registry_.try_get<game::component::BlockerComponent>(entity);
    if (auto name = registry_.try_get<engine::component::NameComponent>(entity); name) {
        ImGui::Text("%s  ", name->name_.c_str());
        ImGui::SameLine();
    }
    ImGui::Text("%s", class_name.class_name_.c_str());
    ImGui::Text("Level: %d", stats.level_);
    ImGui::SameLine();
    ImGui::Text("Rarity: %d", stats.rarity_);
    ImGui::Text("HP: %d/%d", static_cast<int>(std::round(stats.hp_)), static_cast<int>(std::round(stats.max_hp_)));
    ImGui::Text("ATK: %d", static_cast<int>(std::round(stats.atk_)));
    ImGui::SameLine();
    ImGui::Text("DEF: %d", static_cast<int>(std::round(stats.def_)));
    ImGui::Text("Range: %d", static_cast<int>(std::round(stats.range_)));
    ImGui::SameLine();
    ImGui::Text("Interval: %.2f", stats.atk_interval_);
    if (blocker) {
        ImGui::Text("Block: %d/%d", blocker->current_count_, blocker->max_count_);
    }

    // Upgrade, consumption COST same as deployment COST
    const auto& player = registry_.get<game::component::PlayerComponent>(entity);
    auto available_cost = registry_.ctx().get<game::data::GameStats&>().cost_;
    bool button_available = available_cost >= player.cost_;
    // Upgrade button only available when COST is sufficient
    ImGui::BeginDisabled(!button_available);
    // Set hotkey U to upgrade
    ImGui::SetNextItemShortcut(ImGuiKey_U, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
    if (ImGui::Button("Upgrade")) {
        context_.getDispatcher().enqueue<game::defs::UpgradeUnitEvent>(entity, player.cost_);
    }
    ImGui::SameLine();
    ImGui::Text("Hotkey U: COST Cost: %d", player.cost_);
    ImGui::EndDisabled();

    // Retreat, return 50% of COST
    auto return_cost = static_cast<int>(player.cost_ * 0.5f);
    // Set hotkey R to retreat
    ImGui::SetNextItemShortcut(ImGuiKey_R, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
    if (ImGui::Button("Retreat")) {
        context_.getDispatcher().enqueue<game::defs::RetreatEvent>(entity, return_cost);
    }
    ImGui::SameLine();
    ImGui::Text("Hotkey R: COST Return: %d", return_cost);

    // Skill display and interaction
    if (auto skill = registry_.try_get<game::component::SkillComponent>(entity); skill) {
        // If skill ready, button available (activate skill), otherwise unavailable
        auto ready = registry_.all_of<game::defs::SkillReadyTag>(entity);
        ImGui::BeginDisabled(!ready);
        // Set hotkey S to activate skill
        ImGui::SetNextItemShortcut(ImGuiKey_S, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
        if (ImGui::Button(skill->name_.c_str())) {
            // Activate skill
            context_.getDispatcher().enqueue<game::defs::SkillActiveEvent>(entity);
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        // If skill active, show "Remaining time" or "Passive skill active"
        if (registry_.all_of<game::defs::SkillActiveTag>(entity)) {
            if (registry_.all_of<game::defs::PassiveSkillTag>(entity)) {
                ImGui::Text("Passive Active");
            } else {
                ImGui::Text("Active, Time Left: %.1f s", skill->duration_ - skill->duration_timer_);
            }
        // Otherwise show cooldown
        } else {
            ImGui::Text("Hotkey S: ");
            ImGui::SameLine();
            if (registry_.all_of<game::defs::SkillReadyTag>(entity)) {
                ImGui::Text("Ready");
            } else {
                // Show cooldown percentage with progress bar
                ImGui::ProgressBar(skill->cooldown_timer_ / skill->cooldown_);
            }
        }
        // Display skill description
        ImGui::TextWrapped("%s", skill->description_.c_str());
    }
    ImGui::End();
}

void DebugUISystem::renderInfoUI() {
    if (!ImGui::Begin("Level Info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        spdlog::error("Level Info window failed to open");
        return;
    }
    // Get level related data
    const auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    const auto& waves = registry_.ctx().get<game::data::Waves&>();
    const auto& session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    // Display
    ImGui::Text("Base HP: %d / 5", game_stats.home_hp_);
    ImGui::SameLine();
    ImGui::Text("COST: %d", static_cast<int>(game_stats.cost_));
    ImGui::SameLine();
    ImGui::Text("Remaining Waves: %ld", waves.waves_.size());
    ImGui::SameLine();
    if (waves.waves_.size() > 0) {
        ImGui::Text("Next Wave: %d", static_cast<int>(waves.next_wave_count_down_));
    }
    ImGui::SameLine();
    ImGui::Text("Kills: %d / %d", game_stats.enemy_killed_count_, game_stats.enemy_count_);
    ImGui::SameLine();
    ImGui::Text("Current Level: %d", session_data->getLevelNumber());
    ImGui::End();
}

void DebugUISystem::renderSettingUI() {
    if (!ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("Settings window failed to open");
        return;
    }
    // Scene Control
    auto& game_state = context_.getGameState();
    ImGui::SetNextItemShortcut(ImGuiKey_P, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
    if (game_state.isPaused()) {    // If game paused, show "Resume" button, hotkey P
        if (ImGui::Button("Resume")) {
            game_state.setState(engine::core::State::Playing);
        }
    } else {        // If game running, show "Pause" button, hotkey P
        if (ImGui::Button("Pause")) {
            game_state.setState(engine::core::State::Paused);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart")) {
        context_.getDispatcher().enqueue<game::defs::RestartEvent>();
    }
    if (ImGui::Button("Title Screen")) {
        context_.getDispatcher().enqueue<game::defs::BackToTitleEvent>();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        context_.getDispatcher().enqueue<game::defs::SaveEvent>();
    }
    ImGui::Separator();

    // Game Speed Adjustment
    auto& time = context_.getTime();
    float time_scale = time.getTimeScale();
    if (ImGui::Button("0.5x")) {
        time_scale = 0.5f;
        time.setTimeScale(time_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("1x")) {
        time_scale = 1.0f;
        time.setTimeScale(time_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("2x")) {
        time_scale = 2.0f;
        time.setTimeScale(time_scale);
    }
    ImGui::SliderFloat("Game Speed", &time_scale, 0.5f, 2.0f);
    time.setTimeScale(time_scale);

    // Audio Volume Adjustment
    float music_volume = context_.getAudioPlayer().getMusicVolume();
    ImGui::SliderFloat("Music Volume", &music_volume, 0.0f, 1.0f);
    context_.getAudioPlayer().setMusicVolume(music_volume);
    float sound_volume = context_.getAudioPlayer().getSoundVolume();
    ImGui::SliderFloat("SFX Volume", &sound_volume, 0.0f, 1.0f);
    context_.getAudioPlayer().setSoundVolume(sound_volume);

    // Toggle Debug UI
    ImGui::Checkbox("Show Debug Tools", &show_debug_ui_);
    ImGui::End();
}

void DebugUISystem::renderDebugUI() {
    if (!show_debug_ui_) return;
    if (!ImGui::Begin("Debug Tools", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        spdlog::error("Debug Tools window failed to open");
        return;
    }
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    if (ImGui::Button("COST + 10")) {
        game_stats.cost_ += 10;
    }
    if (ImGui::Button("COST + 100")) {
        game_stats.cost_ += 100;
    }
    if (ImGui::Button("Cheat: Clear Level")) {
        context_.getDispatcher().enqueue<game::defs::LevelClearEvent>();
    }
    // TODO: Add other debug tools as needed
    ImGui::End();
}

// ----------------------------- TitleScene -----------------------------
void DebugUISystem::renderTitleLogo() {
    if (!ImGui::Begin("TitleLogo", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground)) {
        ImGui::End();
        spdlog::error("TitleLogo窗口打开失败");
        return;
    }
    // 获取LOGO图片信息
    auto& resource_manager = context_.getResourceManager();
    auto logo_texture = resource_manager.getTexture("assets/textures/UI/title.png"_hs);
    auto size = resource_manager.getTextureSize("assets/textures/UI/title.png"_hs);
    // 图片显示参数：SDL_Texture*, 显示尺寸(像素)。源矩形区域默认为整张图片。
    ImGui::Image(logo_texture, ImVec2(size.x, size.y));
    ImGui::End();
}

void DebugUISystem::renderTitleButtons(game::scene::TitleScene& title_scene) {
    if (!ImGui::Begin("TitleUI", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("TitleUI window failed to open");
        return;
    }
    // Scale font larger for buttons
    ImGui::SetWindowFontScale(2.0f);
    if (ImGui::Button("Start Game", ImVec2(200, 60))) {
        title_scene.onStartGameClick();     // Directly call TitleScene function
    }
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
    if (ImGui::Button("Manage Units", ImVec2(200, 60))) {
        title_scene.onConfirmRoleClick();
    }
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
    if (ImGui::Button("Load Game", ImVec2(200, 60))) {
        title_scene.onLoadGameClick();
    }
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
    if (ImGui::Button("Exit", ImVec2(200, 60))) {
        title_scene.onQuitClick();
    }
    ImGui::SetWindowFontScale(1.0f);    // Restore default font scale
    ImGui::End();
}

// ----------------------------- LevelClearScene -----------------------------
void DebugUISystem::renderLevelClearText() {
    if (!ImGui::Begin("Level Clear Text", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("Level Clear Text window failed to open");
        return;
    }
    ImGui::SetWindowFontScale(3.0f);
    ImGui::Text("VICTORY!");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

void DebugUISystem::renderLevelClearTable(game::scene::LevelClearScene& level_clear_scene) {
    if (!ImGui::Begin("Level Result", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("Level Result window failed to open");
        return;
    }
    renderUnitTable();  // Unit table with sorting support
    ImGui::Separator();
    // Show some clear stats
    const auto& session_info = level_clear_scene.game_stats_;
    const auto& session_data = level_clear_scene.session_data_;
    ImGui::Text("Level: %d", session_data->getLevelNumber());
    ImGui::SameLine();
    ImGui::Text("Kills: %d / %d", session_info.enemy_killed_count_, session_info.enemy_count_);
    ImGui::SameLine();
    ImGui::Text("Base HP: %d / 5", session_info.home_hp_);
    ImGui::SameLine();
    ImGui::Text("Reward Points: %d", session_info.enemy_killed_count_ + session_info.home_hp_ * 5);
    ImGui::SameLine();
    ImGui::Text("Points: %d", session_data->getPoint());
    ImGui::End();
}

void DebugUISystem::renderLevelClearButtons(game::scene::LevelClearScene& level_clear_scene) {
    if (!ImGui::Begin("Level Clear Buttons", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        spdlog::error("Level Clear Buttons window failed to open");
        return;
    }
    ImGui::SetWindowFontScale(1.5f);
    if (ImGui::Button("Next Level", ImVec2(150, 45))) {
        level_clear_scene.onNextLevelClick();
    }
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
    if (ImGui::Button("Save", ImVec2(150, 45))) {
        level_clear_scene.onSaveClick();
    }
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
    if (ImGui::Button("Title Screen", ImVec2(150, 45))) {
        level_clear_scene.onBackToTitleClick();
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

// ----------------------------- EndScene -----------------------------
void DebugUISystem::renderEndText(game::scene::EndScene& end_scene) {
    if (!ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        spdlog::error("Game Over window failed to open");
        return;
    }
    ImGui::SetWindowFontScale(5.0f);
    if (end_scene.is_win_) {
        ImGui::Text("Congratulations, VICTORY!");
    } else {
        ImGui::Text("DEFEAT. Try again!");
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

void DebugUISystem::renderEndButtons(game::scene::EndScene& end_scene) {
    if (!ImGui::Begin("End Buttons", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        spdlog::error("End Buttons window failed to open");
        return;
    }
    ImGui::SetWindowFontScale(1.5f);
    if (ImGui::Button("Title Screen", ImVec2(150, 45))) {
        end_scene.onBackToTitleClick();
    }
    ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
    if (ImGui::Button("Exit", ImVec2(150, 45))) {
        end_scene.onQuitClick();
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

// ----------------------------- Shared -----------------------------
void DebugUISystem::renderUnitInfoUI(bool& show_unit_info) {
    if (!show_unit_info) return;
    // show_unit_info will be set to false when closing the window
    if (!ImGui::Begin("Unit Management", &show_unit_info, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        spdlog::error("Unit Management window failed to open");
        return;
    }
    renderUnitTable();
    ImGui::Separator();
    const auto session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    ImGui::Text("Points: %d", session_data->getPoint());
    ImGui::End();
}

void DebugUISystem::renderLoadPanelUI(bool& show_load_panel) {
    if (!show_load_panel) return;
    if (!ImGui::Begin("Load Game", &show_load_panel, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        spdlog::error("Load Game window failed to open");
        return;
    }
    const auto& session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    if (ImGui::Button("SLOT 1")) {
        session_data->loadFromFile("assets/save/SLOT_1.json");
    }
    ImGui::SameLine();
    if (ImGui::Button("SLOT 2")) {
        session_data->loadFromFile("assets/save/SLOT_2.json");
    }
    ImGui::SameLine();
    if (ImGui::Button("SLOT 3")) {
        session_data->loadFromFile("assets/save/SLOT_3.json");
    }
    // If level cleared, prompt next level, otherwise current level
    if (session_data->isLevelClear()) {
        ImGui::Text("Next Level: %d", session_data->getLevelNumber() + 1);
    } else {
        ImGui::Text("Current Level: %d", session_data->getLevelNumber());
    }
    ImGui::End();
}

void DebugUISystem::renderSavePanelUI(bool& show_save_panel) {
    if (!show_save_panel) return;
    if (!ImGui::Begin("Save Game", &show_save_panel, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        spdlog::error("Save Game window failed to open");
        return;
    }
    const auto& session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    if (ImGui::Button("SLOT 1")) {        
        session_data->saveToFile("assets/save/SLOT_1.json");
    }
    ImGui::SameLine();
    if (ImGui::Button("SLOT 2")) {
        session_data->saveToFile("assets/save/SLOT_2.json");
    }
    ImGui::SameLine();
    if (ImGui::Button("SLOT 3")) {
        session_data->saveToFile("assets/save/SLOT_3.json");
    }
    // If level cleared, prompt next level, otherwise current level
    if (session_data->isLevelClear()) {
        ImGui::Text("Next Level: %d", session_data->getLevelNumber() + 1);
    } else {
        ImGui::Text("Current Level: %d", session_data->getLevelNumber());
    }
    ImGui::End();
}

void DebugUISystem::renderUnitTable() {
    // Show table with 14 columns, use ImGuiTableFlags_Sortable for sorting support
    if (!ImGui::BeginTable("Unit Info", 14, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable)) {
        ImGui::End();
        spdlog::error("Unit Info table failed to open");
        return;
    }
    // Define columns
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Class");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Lv");
    ImGui::TableSetupColumn("Rarity");
    ImGui::TableSetupColumn("COST");
    ImGui::TableSetupColumn("HP");
    ImGui::TableSetupColumn("ATK");
    ImGui::TableSetupColumn("DEF");
    ImGui::TableSetupColumn("Range");
    ImGui::TableSetupColumn("ASPD");
    ImGui::TableSetupColumn("Block");
    ImGui::TableSetupColumn("Skill");
    ImGui::TableSetupColumn("Upgrade");
    // Render header row
    ImGui::TableHeadersRow();
    // 获取数据
    const auto session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    auto& unit_data_list = session_data->getUnitDataList();
    const auto blueprint_manager = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();
    const auto ui_config = registry_.ctx().get<std::shared_ptr<game::data::UIConfig>>();

    // --- 点击标题列，就按照该列排序 ---
    // 获取排序规格参数 (当sort_specs->SpecsDirty为true时，表示点击了某一列标题，需要重新排序)
    if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs()) {
        if (sort_specs->SpecsDirty && !unit_data_list.empty()) {
            // 获取第一个（也是唯一的）排序规格参数 (多列排序会有更多规格参数)
            const ImGuiTableColumnSortSpecs& spec = sort_specs->Specs[0];
            const int col = spec.ColumnIndex;       // 获取列的索引，对应鼠标点击的列
            const bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending);
            
            // 创建字符串比较函数 (如果左<右，返回-1；如果左>右，返回1；如果左==右，返回0)
            const auto compareStrings = [](const std::string& a, const std::string& b) {
                return (a < b) ? -1 : (a > b ? 1 : 0);
            };
            
            // 执行排序
            std::stable_sort(unit_data_list.begin(), unit_data_list.end(), 
                             [&](const game::data::UnitData* lhs, const game::data::UnitData* rhs) {
                // 获取职业蓝图（用于获取属性）
                const auto& pcb_l = blueprint_manager->getPlayerClassBlueprint(lhs->class_id_);
                const auto& pcb_r = blueprint_manager->getPlayerClassBlueprint(rhs->class_id_);
                
                // delta用于记录比较结果（-1表示小于，0表示等于，1表示大于）
                int delta = 0;

                switch (col) {
                    case 0: {   // Name
                        delta = compareStrings(lhs->name_, rhs->name_);
                        break;
                    }
                    case 1: {   // Class
                        delta = compareStrings(lhs->class_, rhs->class_);
                        break;
                    }
                    case 2: {   // Type (Melee, Ranged, Mixed)
                        const int type_l = static_cast<int>(pcb_l.player_.type_);
                        const int type_r = static_cast<int>(pcb_r.player_.type_);
                        delta = (type_l < type_r) ? -1 : (type_l > type_r ? 1 : 0);
                        break;
                    }
                    case 3: {   // Level
                        delta = (lhs->level_ < rhs->level_) ? -1 : (lhs->level_ > rhs->level_ ? 1 : 0);
                        break;
                    }
                    case 4: {   // Rarity
                        delta = (lhs->rarity_ < rhs->rarity_) ? -1 : (lhs->rarity_ > rhs->rarity_ ? 1 : 0);
                        break;
                    }
                    case 5: {   // COST
                        const int cost_l = static_cast<int>(std::round(engine::utils::statModify(pcb_l.player_.cost_, 1, lhs->rarity_)));
                        const int cost_r = static_cast<int>(std::round(engine::utils::statModify(pcb_r.player_.cost_, 1, rhs->rarity_)));
                        delta = (cost_l < cost_r) ? -1 : (cost_l > cost_r ? 1 : 0);
                        break;
                    }
                    case 6: {   // HP
                        const int hp_l = static_cast<int>(std::round(engine::utils::statModify(pcb_l.stats_.hp_, lhs->level_, lhs->rarity_)));
                        const int hp_r = static_cast<int>(std::round(engine::utils::statModify(pcb_r.stats_.hp_, rhs->level_, rhs->rarity_)));
                        delta = (hp_l < hp_r) ? -1 : (hp_l > hp_r ? 1 : 0);
                        break;
                    }
                    case 7: {   // ATK
                        const int atk_l = static_cast<int>(std::round(engine::utils::statModify(pcb_l.stats_.atk_, lhs->level_, lhs->rarity_)));
                        const int atk_r = static_cast<int>(std::round(engine::utils::statModify(pcb_r.stats_.atk_, rhs->level_, rhs->rarity_)));
                        delta = (atk_l < atk_r) ? -1 : (atk_l > atk_r ? 1 : 0);
                        break;
                    }
                    case 8: {   // DEF
                        const int def_l = static_cast<int>(std::round(engine::utils::statModify(pcb_l.stats_.def_, lhs->level_, lhs->rarity_)));
                        const int def_r = static_cast<int>(std::round(engine::utils::statModify(pcb_r.stats_.def_, rhs->level_, rhs->rarity_)));
                        delta = (def_l < def_r) ? -1 : (def_l > def_r ? 1 : 0);
                        break;
                    }
                    case 9: {   // Range
                        const int range_l = static_cast<int>(std::round(pcb_l.stats_.range_));
                        const int range_r = static_cast<int>(std::round(pcb_r.stats_.range_));
                        delta = (range_l < range_r) ? -1 : (range_l > range_r ? 1 : 0);
                        break;
                    }
                    case 10: {  // ASPD (Attack Interval)
                        const float ai_l = pcb_l.stats_.atk_interval_;
                        const float ai_r = pcb_r.stats_.atk_interval_;
                        delta = (ai_l < ai_r) ? -1 : (ai_l > ai_r ? 1 : 0);
                        break;
                    }
                    case 11: {  // Block Count
                        delta = (pcb_l.player_.block_ < pcb_r.player_.block_) ? -1 : (pcb_l.player_.block_ > pcb_r.player_.block_ ? 1 : 0);
                        break;
                    }
                    case 12: {  // Skill
                        const auto& sk_l = blueprint_manager->getSkillBlueprint(pcb_l.player_.skill_id_);
                        const auto& sk_r = blueprint_manager->getSkillBlueprint(pcb_r.player_.skill_id_);
                        delta = compareStrings(sk_l.name_, sk_r.name_);
                        break;
                    }
                    case 13: {  // Upgrade Button (Consistent with COST sorting)
                        const int cost_l = static_cast<int>(std::round(engine::utils::statModify(pcb_l.player_.cost_, 1, lhs->rarity_)));
                        const int cost_r = static_cast<int>(std::round(engine::utils::statModify(pcb_r.player_.cost_, 1, rhs->rarity_)));
                        delta = (cost_l < cost_r) ? -1 : (cost_l > cost_r ? 1 : 0);
                        break;
                    }
                    default: break;
                }

                // 根据升序还是降序返回比较结果
                return ascending ? (delta < 0) : (delta > 0);
            });
            
            // 完成排序后，将SpecsDirty设置为false，下轮更新会跳过排序操作（即脏标识模式）
            sort_specs->SpecsDirty = false;
        }
    }

    // 渲染数据行
    for (const auto& unit : unit_data_list) {
        // 获取并计算属性数据信息
        const auto& player_class_blueprint = blueprint_manager->getPlayerClassBlueprint(unit->class_id_);
        const auto& skill_blueprint = blueprint_manager->getSkillBlueprint(player_class_blueprint.player_.skill_id_);
        const auto& stats = player_class_blueprint.stats_;
        const auto hp = engine::utils::statModify(stats.hp_, unit->level_, unit->rarity_);
        const auto atk = engine::utils::statModify(stats.atk_, unit->level_, unit->rarity_);
        const auto def = engine::utils::statModify(stats.def_, unit->level_, unit->rarity_);
        const auto cost = engine::utils::statModify(player_class_blueprint.player_.cost_, 1, unit->rarity_);
        std::string type = player_class_blueprint.player_.type_ == game::defs::PlayerType::MELEE ? "Melee" : 
            player_class_blueprint.player_.type_ == game::defs::PlayerType::RANGED ? "Ranged" : 
            player_class_blueprint.player_.type_ == game::defs::PlayerType::MIXED ? "Mixed" : "Unknown";

        // 获取头像信息
        const auto& portrait_image = ui_config->getPortrait(unit->name_id_);
        auto portrait_texture = context_.getResourceManager().getTexture(portrait_image.getTextureId(), portrait_image.getTexturePath());
        auto portrait_rect = portrait_image.getSourceRect();  // 源矩形的区域
        auto sprite_sheet_size = context_.getResourceManager().getTextureSize(portrait_image.getTextureId());   // 获取精灵图的尺寸

        // 计算头像的UV坐标（即源矩形左上、右下的坐标，相对于整张精灵图大小的比例，取值在0～1之间）
        float u = portrait_rect->position.x / sprite_sheet_size.x;
        float v = portrait_rect->position.y / sprite_sheet_size.y;
        float u2 = (portrait_rect->position.x + portrait_rect->size.x) / sprite_sheet_size.x;
        float v2 = (portrait_rect->position.y + portrait_rect->size.y) / sprite_sheet_size.y;

        // 设置显示尺寸
        constexpr glm::vec2 DISPLAY_SIZE = glm::vec2(128.0f, 128.0f);

        // 新建一行
        ImGui::TableNextRow();
        // 每一行依次填充对应列的信息
        ImGui::TableNextColumn();       // 第一列：姓名
        ImGui::Text("%s", unit->name_.c_str());
        // 如果鼠标悬浮在该UI组件上，显示复杂信息（支持各种UI组件及其组合，例如下面的Tooltip组件）
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            // 图片显示参数：SDL_Texture*, 显示尺寸(像素), 源矩形的左上角UV坐标，源矩形的右下角UV坐标
            ImGui::Image(portrait_texture, ImVec2(DISPLAY_SIZE.x, DISPLAY_SIZE.y), ImVec2(u, v), ImVec2(u2, v2));
            ImGui::EndTooltip();
        }
        ImGui::TableNextColumn();       // 第二列：职业
        ImGui::Text("%s", player_class_blueprint.display_info_.name_.c_str());
        // 如果鼠标悬浮在该UI组件上，显示描述信息（只支持文本）
        ImGui::SetItemTooltip("%s", player_class_blueprint.display_info_.description_.c_str());
        ImGui::TableNextColumn();       // 第三列：类型
        ImGui::Text("%s", type.c_str());
        ImGui::TableNextColumn();       // 第四列：等级
        ImGui::Text("%d", unit->level_);
        ImGui::TableNextColumn();       // 第五列：稀有度
        ImGui::Text("%d", unit->rarity_);
        ImGui::TableNextColumn();       // 第六列：COST
        ImGui::Text("%d", static_cast<int>(std::round(cost)));
        ImGui::TableNextColumn();       // 第七列：生命值
        ImGui::Text("%d", static_cast<int>(std::round(hp)));
        ImGui::TableNextColumn();        // 第八列：攻击力
        ImGui::Text("%d", static_cast<int>(std::round(atk)));
        ImGui::TableNextColumn();        // 第九列：防御力
        ImGui::Text("%d", static_cast<int>(std::round(def)));
        ImGui::TableNextColumn();        // 第十列：攻击范围
        ImGui::Text("%d", static_cast<int>(std::round(player_class_blueprint.stats_.range_)));
        ImGui::TableNextColumn();        // 第十一列：攻击间隔
        ImGui::Text("%.2f", player_class_blueprint.stats_.atk_interval_);
        ImGui::TableNextColumn();        // 第十二列：阻挡数量
        ImGui::Text("%d", player_class_blueprint.player_.block_);
        ImGui::TableNextColumn();        // 第十三列：技能
        ImGui::Text("%s", skill_blueprint.name_.c_str());
        ImGui::SetItemTooltip("%s", skill_blueprint.description_.c_str());
        ImGui::TableNextColumn();        // 第十四列：升级按钮

        // 使用 name_ 作为下一个UI组件(即Button)的 ID，确保唯一性，否则同名Button会冲突
        ImGui::PushID(unit->name_.c_str());
            // 根据积分点数，判断是否可以升级，并决定升级按钮是否可用
        bool can_upgrade = session_data->getPoint() >= static_cast<int>(std::round(cost));
        ImGui::BeginDisabled(!can_upgrade);
        std::string button_text = "- " + std::to_string(static_cast<int>(std::round(cost)));
        if (ImGui::Button(button_text.c_str())) {   // 如果没有PushID，默认会以Button中显示参数作为ID，那会出现重复ID
            session_data->addPoint(-static_cast<int>(std::round(cost)));
            unit->level_ += 1;
        }
        ImGui::EndDisabled();
        ImGui::PopID();     // Match with PushID above
        ImGui::SetItemTooltip("Points needed to upgrade: %d", static_cast<int>(std::round(cost)));
    }
    ImGui::EndTable();
}

// 事件回调函数
void DebugUISystem::onUIPortraitHoverEnterEvent(const game::defs::UIPortraitHoverEnterEvent& event) {
    hovered_portrait_ = event.name_id_;
}
void DebugUISystem::onUIPortraitHoverLeaveEvent() {
    hovered_portrait_ = entt::null;
}

} // namespace game::system