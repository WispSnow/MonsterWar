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
#include "../../engine/audio/audio_player.h"
#include "../../engine/component/name_component.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/core/time.h"
#include "../../engine/render/renderer.h"
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

    // 显示Tooltip信息
    if (!ImGui::BeginTooltip()) {
        ImGui::EndTooltip();
        spdlog::error("鼠标悬浮单位肖像窗口打开失败");
        return;
    }
    ImGui::Text("%s", unit_data.name_.c_str());
    ImGui::SameLine();
    ImGui::Text("职业: %s", name.data());
    ImGui::Text("等级: %d", unit_data.level_);
    ImGui::SameLine();
    ImGui::Text("稀有度: %d", unit_data.rarity_);
    ImGui::Text("生命值: %d", static_cast<int>(std::round(hp)));
    ImGui::SameLine();
    ImGui::Text("攻击力: %d", static_cast<int>(std::round(atk)));
    ImGui::Text("防御力: %d", static_cast<int>(std::round(def)));
    ImGui::SameLine();
    ImGui::Text("攻击范围: %d", static_cast<int>(std::round(range)));
    ImGui::EndTooltip();
}

void DebugUISystem::renderHoveredUnit() {
    // 确定鼠标悬浮的单位存在
    auto& entity = registry_.ctx().get<entt::entity&>("hovered_unit"_hs);
    if (entity == entt::null || !registry_.valid(entity)) return;

    // Tooltip 是悬浮在鼠标上的小窗口，可以显示单位信息
    if (!ImGui::BeginTooltip()) {
        ImGui::EndTooltip();
        spdlog::error("鼠标悬浮单位窗口打开失败");
        return;
    }
    // 获取必要信息并显示
    const auto& stats = registry_.get<game::component::StatsComponent>(entity);
    const auto& class_name = registry_.get<game::component::ClassNameComponent>(entity);
    // 只有玩家单位才有姓名，所以需要尝试获取
    if (auto name = registry_.try_get<engine::component::NameComponent>(entity); name) {
        ImGui::Text("%s  ", name->name_.c_str());
        ImGui::SameLine();
    }
    ImGui::Text("%s", class_name.class_name_.c_str());
    ImGui::Text("等级: %d", stats.level_);
    ImGui::SameLine();
    ImGui::Text("稀有度: %d", stats.rarity_);
    ImGui::Text("生命值: %d/%d", static_cast<int>(std::round(stats.hp_)), static_cast<int>(std::round(stats.max_hp_)));
    ImGui::Text("攻击力: %d", static_cast<int>(std::round(stats.atk_)));
    ImGui::Text("防御力: %d", static_cast<int>(std::round(stats.def_)));
    ImGui::Text("攻击范围: %d", static_cast<int>(std::round(stats.range_)));
    ImGui::Text("攻击间隔: %.2f", stats.atk_interval_);
    ImGui::EndTooltip();
}

void DebugUISystem::renderSelectedUnit() {
    // 确定选中的单位存在
    auto& entity = registry_.ctx().get<entt::entity&>("selected_unit"_hs);
    if (entity == entt::null || !registry_.valid(entity)) return;

    // 设置窗口位置在左上角
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);

    if (!ImGui::Begin("角色状态", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("角色状态窗口打开失败");
        return;
    }
    // 获取必要信息并显示
    const auto& stats = registry_.get<game::component::StatsComponent>(entity);
    const auto& class_name = registry_.get<game::component::ClassNameComponent>(entity);
    const auto blocker = registry_.try_get<game::component::BlockerComponent>(entity);
    if (auto name = registry_.try_get<engine::component::NameComponent>(entity); name) {
        ImGui::Text("%s  ", name->name_.c_str());
        ImGui::SameLine();
    }
    ImGui::Text("%s", class_name.class_name_.c_str());
    ImGui::Text("等级: %d", stats.level_);
    ImGui::SameLine();
    ImGui::Text("稀有度: %d", stats.rarity_);
    ImGui::Text("生命值: %d/%d", static_cast<int>(std::round(stats.hp_)), static_cast<int>(std::round(stats.max_hp_)));
    ImGui::Text("攻击力: %d", static_cast<int>(std::round(stats.atk_)));
    ImGui::SameLine();
    ImGui::Text("防御力: %d", static_cast<int>(std::round(stats.def_)));
    ImGui::Text("攻击范围: %d", static_cast<int>(std::round(stats.range_)));
    ImGui::SameLine();
    ImGui::Text("攻击间隔: %.2f", stats.atk_interval_);
    if (blocker) {
        ImGui::Text("阻挡数量: %d/%d", blocker->current_count_, blocker->max_count_);
    }

    // 升级，消耗COST与出击COST相同
    const auto& player = registry_.get<game::component::PlayerComponent>(entity);
    auto available_cost = registry_.ctx().get<game::data::GameStats&>().cost_;
    bool button_available = available_cost >= player.cost_;
    // COST资源充足时升级按钮才可用
    ImGui::BeginDisabled(!button_available);
    // 设置快捷键 U 升级
    ImGui::SetNextItemShortcut(ImGuiKey_U, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
    if (ImGui::Button("升级")) {
        context_.getDispatcher().enqueue<game::defs::UpgradeUnitEvent>(entity, player.cost_);
    }
    ImGui::SameLine();
    ImGui::Text("快捷键 U: COST消费: %d", player.cost_);
    ImGui::EndDisabled();

    // 撤退，返回 50% 的COST
    auto return_cost = static_cast<int>(player.cost_ * 0.5f);
    // 设置快捷键 R 撤退
    ImGui::SetNextItemShortcut(ImGuiKey_R, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
    if (ImGui::Button("撤退")) {
        context_.getDispatcher().enqueue<game::defs::RetreatEvent>(entity, return_cost);
    }
    ImGui::SameLine();
    ImGui::Text("快捷键 R: COST返还: %d", return_cost);

    // 技能显示与交互
    if (auto skill = registry_.try_get<game::component::SkillComponent>(entity); skill) {
        // 如果技能准备就绪，则按钮可用（激活技能），否则按钮不可用
        auto ready = registry_.all_of<game::defs::SkillReadyTag>(entity);
        ImGui::BeginDisabled(!ready);
        // 设置快捷键 S 激活技能
        ImGui::SetNextItemShortcut(ImGuiKey_S, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
        if (ImGui::Button(skill->name_.c_str())) {
            // 激活技能
            context_.getDispatcher().enqueue<game::defs::SkillActiveEvent>(entity);
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        // 如果技能激活中，则显示"剩余时间"或"被动技能激活中"
        if (registry_.all_of<game::defs::SkillActiveTag>(entity)) {
            if (registry_.all_of<game::defs::PassiveSkillTag>(entity)) {
                ImGui::Text("被动技能激活中");
            } else {
                ImGui::Text("激活中，剩余时间: %.1f 秒", skill->duration_ - skill->duration_timer_);
            }
        // 否则显示冷却时间
        } else {
            ImGui::Text("快捷键 S: ");
            ImGui::SameLine();
            if (registry_.all_of<game::defs::SkillReadyTag>(entity)) {
                ImGui::Text("技能准备就绪");
            } else {
                // 用进度条显示冷却时间百分比
                ImGui::ProgressBar(skill->cooldown_timer_ / skill->cooldown_);
            }
        }
        // 显示技能描述
        ImGui::TextWrapped("%s", skill->description_.c_str());
    }
    ImGui::End();
}

void DebugUISystem::renderInfoUI() {
    if (!ImGui::Begin("关卡信息", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        spdlog::error("关卡信息窗口打开失败");
        return;
    }
    // 获取关卡相关数据
    const auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    const auto& waves = registry_.ctx().get<game::data::Waves&>();
    const auto& session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    // 显示
    ImGui::Text("基地血量: %d / 5", game_stats.home_hp_);
    ImGui::SameLine();
    ImGui::Text("COST: %d", static_cast<int>(game_stats.cost_));
    ImGui::SameLine();
    ImGui::Text("剩余波次: %ld", waves.waves_.size());
    ImGui::SameLine();
    if (waves.waves_.size() > 0) {
        ImGui::Text("下一波时间: %d", static_cast<int>(waves.next_wave_count_down_));
    }
    ImGui::SameLine();
    ImGui::Text("击杀数量: %d / %d", game_stats.enemy_killed_count_, game_stats.enemy_count_);
    ImGui::SameLine();
    ImGui::Text("当前关卡: %d", session_data->getLevelNumber());
    ImGui::End();
}

void DebugUISystem::renderSettingUI() {
    if (!ImGui::Begin("设置工具", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("设置工具窗口打开失败");
        return;
    }
    // 场景控制
    auto& game_state = context_.getGameState();
    ImGui::SetNextItemShortcut(ImGuiKey_P, ImGuiInputFlags_RouteAlways | ImGuiInputFlags_Tooltip);
    if (game_state.isPaused()) {    // 如果游戏暂停，则显示“继续游戏”按钮，快捷键 P
        if (ImGui::Button("继续游戏")) {
            game_state.setState(engine::core::State::Playing);
        }
    } else {        // 如果游戏运行中，则显示“暂停游戏”按钮，快捷键也是 P
        if (ImGui::Button("暂停游戏")) {
            game_state.setState(engine::core::State::Paused);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("重新开始")) {
        context_.getDispatcher().enqueue<game::defs::RestartEvent>();
    }
    if (ImGui::Button("返回标题")) {
        context_.getDispatcher().enqueue<game::defs::BackToTitleEvent>();
    }
    ImGui::SameLine();
    if (ImGui::Button("保存")) {
        context_.getDispatcher().enqueue<game::defs::SaveEvent>();
    }
    ImGui::Separator();

    // 游戏速度调节
    auto& time = context_.getTime();
    float time_scale = time.getTimeScale();
    if (ImGui::Button("0.5倍速")) {
        time_scale = 0.5f;
        time.setTimeScale(time_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("1倍速")) {
        time_scale = 1.0f;
        time.setTimeScale(time_scale);
    }
    ImGui::SameLine();
    if (ImGui::Button("2倍速")) {
        time_scale = 2.0f;
        time.setTimeScale(time_scale);
    }
    ImGui::SliderFloat("游戏速度", &time_scale, 0.5f, 2.0f);
    time.setTimeScale(time_scale);

    // 音乐音量调节
    float music_volume = context_.getAudioPlayer().getMusicVolume();
    ImGui::SliderFloat("音乐音量", &music_volume, 0.0f, 1.0f);
    context_.getAudioPlayer().setMusicVolume(music_volume);
    float sound_volume = context_.getAudioPlayer().getSoundVolume();
    ImGui::SliderFloat("音效音量", &sound_volume, 0.0f, 1.0f);
    context_.getAudioPlayer().setSoundVolume(sound_volume);

    // 切换调试工具显示 （勾选结果保存在show_debug_ui_中）
    ImGui::Checkbox("显示调试工具", &show_debug_ui_);
    ImGui::End();
}

void DebugUISystem::renderDebugUI() {
    if (!show_debug_ui_) return;
    if (!ImGui::Begin("调试工具", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        spdlog::error("调试工具窗口打开失败");
        return;
    }
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    if (ImGui::Button("COST + 10")) {
        game_stats.cost_ += 10;
    }
    if (ImGui::Button("COST + 100")) {
        game_stats.cost_ += 100;
    }
    if (ImGui::Button("通关")) {
        context_.getDispatcher().enqueue<game::defs::LevelClearEvent>();
    }
    // TODO: 未来可按需添加其他调试工具
    ImGui::End();
}

// 事件回调函数
void DebugUISystem::onUIPortraitHoverEnterEvent(const game::defs::UIPortraitHoverEnterEvent& event) {
    hovered_portrait_ = event.name_id_;
}
void DebugUISystem::onUIPortraitHoverLeaveEvent() {
    hovered_portrait_ = entt::null;
}

} // namespace game::system