#pragma once
#include <glm/vec2.hpp>
#include <vector>

namespace game::data {

/**
 * @brief 路径节点数据结构。
 * @note 包含节点ID、坐标和指向下一个节点的ID列表。
 */
struct WaypointNode {
    int id_;
    glm::vec2 position_;
    std::vector<int> next_node_ids_;    // 指向下一个节点的ID列表
};

}