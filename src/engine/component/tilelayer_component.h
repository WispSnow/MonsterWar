#pragma once
#include "animation_component.h"
#include "sprite_component.h"
#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <utility>
#include <optional>
#include <SDL3/SDL_rect.h>
#include <nlohmann/json.hpp>

namespace engine::component {

/**
 * @brief 定义瓦片的类型，用于游戏逻辑（例如碰撞）。
 * @note 当前项目并未用到此信息
 */
 enum class TileType {
    EMPTY,      ///< @brief 空白瓦片
    NORMAL,     ///< @brief 普通瓦片
    SOLID,      ///< @brief 静止可碰撞瓦片
    HAZARD,     ///< @brief 危险瓦片（例如火焰、尖刺等）
    // 未来补充其它类型
};

/**
 * @brief 瓦片信息，包含精灵、类型、动画和属性。
 * @note 它只是辅助LevelLoader解析的临时数据，不会保存在游戏中。
 */
struct TileInfo {
    engine::component::Sprite sprite_;                      ///< @brief 精灵
    engine::component::TileType type_;                      ///< @brief 类型
    std::optional<engine::component::Animation> animation_; ///< @brief 动画（支持Tiled动画图块）
    std::optional<nlohmann::json> properties_;              ///< @brief 属性（存放自定义属性，方便LevelLoader解析）

    TileInfo() = default;

    TileInfo(engine::component::Sprite sprite, 
             engine::component::TileType type, 
             std::optional<engine::component::Animation> animation = std::nullopt, 
             std::optional<nlohmann::json> properties = std::nullopt) : 
             sprite_(std::move(sprite)), 
             type_(type), 
             animation_(std::move(animation)), 
             properties_(std::move(properties)) {}
};

/**
 * @brief 瓦片层组件，包含瓦片大小、地图大小和瓦片实体列表。
 * @note 现在瓦片层更像一个容器，只是存储所有的“瓦片”，而每个瓦片就是一个实体。
 */
struct TileLayerComponent {
    glm::ivec2 tile_size_;              ///< @brief 瓦片大小
    glm::ivec2 map_size_;               ///< @brief 地图大小
    std::vector<entt::entity> tiles_;   ///< @brief 瓦片实体列表，每个瓦片对应一个实体，按顺序排列

    /**
     * @brief 构造函数
     * @param tile_size 瓦片大小
     * @param map_size 地图大小
     * @param tiles 瓦片实体列表
     */
    TileLayerComponent(glm::ivec2 tile_size, 
                       glm::ivec2 map_size, 
                       std::vector<entt::entity> tiles) : 
                       tile_size_(std::move(tile_size)), 
                       map_size_(std::move(map_size)),
                       tiles_(std::move(tiles)) {}
};

}