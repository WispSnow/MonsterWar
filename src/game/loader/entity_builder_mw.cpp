#include "entity_builder_mw.h"
#include "../../engine/core/context.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace game::loader
{

EntityBuilderMW::EntityBuilderMW(engine::loader::LevelLoader& level_loader, 
                                engine::core::Context& context, 
                                entt::registry& registry, 
                                std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes,
                                std::vector<int>& start_points)
    : engine::loader::BasicEntityBuilder(level_loader, context, registry), waypoint_nodes_(waypoint_nodes), start_points_(start_points)
{}

EntityBuilderMW* EntityBuilderMW::build() {
    if (object_json_ && !tile_info_) {  // 代表自己绘制的形状,当前游戏只用到了路径节点
        buildPath();
    } else {
        BasicEntityBuilder::build();
    }
    
    return this;
}

void EntityBuilderMW::buildPath() {
    // 检查数据有效性
    if (object_json_->value("point", false) != true) return;
    if (!object_json_->contains("properties") || !object_json_->at("properties").is_array()) return;
    auto id = object_json_->value("id", 0);
    if (id == 0) return;

    // 解析数据并添加到容器
    auto position = glm::vec2(object_json_->value("x", 0.0f), object_json_->value("y", 0.0f));
    std::vector<int> next_node_ids;
    for (auto& property : object_json_->at("properties")) {
        // 如果是对象类型，且名称以 next 开头，则添加到 next_node_ids
        if (property.value("type", "") == "object" && property.value("name", "").starts_with("next")) {
            auto next_node_id = property.value("value", 0);
            if (next_node_id != 0) {
                next_node_ids.push_back(next_node_id);
            }
        }
        // 如果名称是 start，且值为真，则将自身id添加到 start_points_ 中
        if (property.value("name", "") == "start" && property.value("value", false) == true) {
            start_points_.push_back(id);
        }
    }
    // 添加到节点容器中
    waypoint_nodes_[id] = game::data::WaypointNode{id, std::move(position), std::move(next_node_ids)};
    spdlog::trace("waypoint_nodes_ size: {}", waypoint_nodes_.size());
}

}   // namespace game::loader


