#pragma once
#include "../../engine/loader/basic_entity_builder.h"
#include "../data/waypoint_node.h"
#include <unordered_map>
#include <vector>

namespace game::loader {

/**
 * @brief 拓展的关卡载入实体生成器
 * 功能包括：
 * 1. 生成路径节点和起点。
 * 2. ...
 */
class EntityBuilderMW final: public engine::loader::BasicEntityBuilder
{   
private:
    // 保存路径节点和起点数据（非拥有）
    std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes_;
    std::vector<int>& start_points_;

public:
    /**
     * @brief 构造函数
     * @param level_loader 关卡载入器
     * @param context 上下文
     * @param registry 实体注册表
     * @param waypoint_nodes 路径节点
     * @param start_points 起点
     */
    EntityBuilderMW(engine::loader::LevelLoader& level_loader, 
                    engine::core::Context& context, 
                    entt::registry& registry, 
                    std::unordered_map<int, game::data::WaypointNode>& waypoint_nodes,
                    std::vector<int>& start_points);
    ~EntityBuilderMW() = default;

    EntityBuilderMW* build() override;

private:
    void buildPath();       ///< @brief 生成路径节点
};

}   // namespace game::loader


