#include "units_portrait_ui.h"
#include "../data/ui_config.h"
#include "../data/session_data.h"
#include "../data/game_stats.h"
#include "../factory/blueprint_manager.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/ui/ui_element.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/ui/ui_label.h"
#include "../../engine/ui/ui_manager.h"
#include <entt/core/hashed_string.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::ui {

UnitsPortraitUI::UnitsPortraitUI(entt::registry& registry, 
    engine::ui::UIManager& ui_manager, 
    engine::core::Context& context)
    : registry_(registry), ui_manager_(ui_manager), context_(context) {
    // 构造函数中直接初始化（创建单位肖像UI），可省去init函数
    createUnitsPortraitUI();
    // 注册事件
    context_.getDispatcher().sink<game::defs::RemoveUIPortraitEvent>().connect<&UnitsPortraitUI::onRemoveUIPortraitEvent>(this);
    spdlog::trace("UnitsPortraitUI 构造完成。");
}

UnitsPortraitUI::~UnitsPortraitUI() {
    context_.getDispatcher().sink<game::defs::RemoveUIPortraitEvent>().disconnect<&UnitsPortraitUI::onRemoveUIPortraitEvent>(this);
}

void UnitsPortraitUI::update(float) {
    updatePortraitCover();
}

void UnitsPortraitUI::updatePortraitCover() {
    // 获取game_stats
    auto& game_stats = registry_.ctx().get<game::data::GameStats&>();
    // 获取anchor_panel中的所有子元素(frame_panel)
    auto& frame_panels = anchor_panel_->getChildren();
    for (auto& frame_panel : frame_panels) {
        // 获取frame_panel中的cover_panel
        auto cover_panel = frame_panel->getChildById("cover_panel"_hs);
        // 设置cover_panel的可见性（frame_panel的order_index_已设为出击cost耗费值）
        if (cover_panel) {
            cover_panel->setVisible(game_stats.cost_ < frame_panel->getOrderIndex());
        }
    }
}

void UnitsPortraitUI::createUnitsPortraitUI() {
    if (!ui_manager_.init(context_.getGameState().getLogicalSize())) return;

    // 获取ui_config、session_data、blueprint_manager上下文数据
    auto ui_config = registry_.ctx().get<std::shared_ptr<game::data::UIConfig>>();
    auto session_data = registry_.ctx().get<std::shared_ptr<game::data::SessionData>>();
    auto blueprint_manager = registry_.ctx().get<std::shared_ptr<game::factory::BlueprintManager>>();

    // 获取单位面板的间隔、角色map、角色数量
    auto padding = ui_config->getUnitPanelPadding();
    auto& unit_map = session_data->getUnitMap();
    auto unit_num = unit_map.size();

    // --- 在屏幕下方创建一个panel UI 条，用于显示角色肖像 ---
    // 获取窗口大小和角色肖像框大小
    auto window_size = context_.getGameState().getLogicalSize();
    auto frame_size = ui_config->getUnitPanelFrameSize();
    // 根据角色数量、角色肖像框大小、间隔计算panel的位置和大小
    auto pos = glm::vec2(0.0f, window_size.y - frame_size.y - 2 * padding);
    auto size = glm::vec2(unit_num*frame_size.x + (unit_num + 1)*padding, frame_size.y + 2 * padding);
    auto anchor_panel = std::make_unique<engine::ui::UIPanel>(pos, size);
    // 设置背景色
    anchor_panel->setBackgroundColor(engine::utils::FColor(0.1f, 0.1f, 0.1f, 0.1f));
    // 设置ID，以后即可根据ID找到该panel
    anchor_panel->setId("anchor_panel"_hs);

    // 依次添加角色肖像，每个肖像显示由四部分依次叠加：portrait，frame，icon，cost，可以通过一个frame_panel定位（位于上层anchor_panel之中）
    int index = 0;
    for (auto& [name_id, unit_data] : unit_map) {
        auto portrait = ui_config->getPortrait(name_id);
        auto frame = ui_config->getPortraitFrame(unit_data.rarity_);
        auto icon = ui_config->getIcon(unit_data.class_id_);
        auto cost = blueprint_manager->getPlayerClassBlueprint(unit_data.class_id_).player_.cost_;
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
            frame_size,
            [this, name_id, &unit_data, cost]() {   // 按钮点击回调：发送单位准备事件
                context_.getDispatcher().enqueue(game::defs::PrepUnitEvent{name_id, unit_data.class_id_, cost}); 
            }
            // TODO: 悬浮进入和悬浮离开回调函数
        ));
        frame_panel->addChild(std::make_unique<engine::ui::UIImage>(icon, glm::vec2(0.0f, 0.0f), frame_size / 2.0f));
        frame_panel->addChild(std::make_unique<engine::ui::UILabel>(context_.getTextRenderer(), 
            std::to_string(cost), 
            ui_config->getUnitPanelFontPath(), 
            ui_config->getUnitPanelFontSize(), 
            engine::utils::FColor::yellow(), 
            ui_config->getUnitPanelFontOffset()
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
    // 将anchor_panel添加到ui_manager中
    ui_manager_.addElement(std::move(anchor_panel));
    
    // 移动赋值之后需要找到anchor_panel，并将指针赋值给成员变量anchor_panel_
    anchor_panel_ = static_cast<engine::ui::UIPanel*>(ui_manager_.getRootElement()->getChildById("anchor_panel"_hs));
    
    anchor_panel_->sortChildrenByOrderIndex();  // 对anchor_panel中的子元素(frame_panel)进行排序
    arrangeUnitsPortraitUI();                   // 按顺序排列anchor_panel中的子元素(frame_panel)的位置 
}

void UnitsPortraitUI::arrangeUnitsPortraitUI() {
    // 获取ui_config
    auto ui_config = registry_.ctx().get<std::shared_ptr<game::data::UIConfig>>();
    // 获取单位面板的间隔、大小
    auto padding = ui_config->getUnitPanelPadding();
    auto frame_size = ui_config->getUnitPanelFrameSize();
    // 遍历panel中的子元素(定位panel)，并依次设定位置
    for (size_t i = 0; i < anchor_panel_->getChildren().size(); i++) {
        auto& child = anchor_panel_->getChildren()[i];
        child->setPosition(glm::vec2(padding + i * (frame_size.x + padding), padding));
    }
    // 更新panel的size
    anchor_panel_->setSize(glm::vec2(padding + anchor_panel_->getChildren().size() * (frame_size.x + padding), 
                                    frame_size.y + 2 * padding));
}

void UnitsPortraitUI::onRemoveUIPortraitEvent(const game::defs::RemoveUIPortraitEvent& event) {
    anchor_panel_->removeChildById(event.name_id_);
    arrangeUnitsPortraitUI();
}

} // namespace game::ui