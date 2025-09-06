#pragma once
#include <optional>
#include <nlohmann/json_fwd.hpp>
#include <glm/vec2.hpp>
#include <entt/entity/registry.hpp>

namespace engine::core {
class Context;
}

namespace engine::component {
    struct TileInfo;
}

namespace engine::loader {
    class LevelLoader;

/**
 * 基础游戏实体生成器，由LevelLoader使用，通过建造者模式生成各种游戏实体
 * 子类可以继承并添加新的buildXXX方法，以实现更多功能
 */
class BasicEntityBuilder {
protected:

    engine::loader::LevelLoader& level_loader_;
    engine::core::Context& context_;
    entt::registry& registry_;
    

    // 解析游戏对象所需要的关键信息
    const nlohmann::json* object_json_ = nullptr;               ///< @brief 来自.tmj地图文件的对象数据
    const engine::component::TileInfo* tile_info_ = nullptr;    ///< @brief 来自.tsj的瓦片数据
    int index_ = -1;                                            ///< @brief 瓦片索引，用于计算位置（瓦片层）

    // --- 保存会多次用到的变量，避免重复解析 ---
    entt::entity entity_id_;
    glm::vec2 position_;
    glm::vec2 dst_size_;
    glm::vec2 src_size_;

public:
    BasicEntityBuilder(engine::loader::LevelLoader& level_loader, engine::core::Context& context, entt::registry& registry);
    virtual ~BasicEntityBuilder();

    // --- 三个关键方法：配置、构建、返回 ---
    /// @brief 针对自定义形状（对象层）
    BasicEntityBuilder* configure(const nlohmann::json* object_json);

    /// @brief 针对图片对象 (对象层)
    BasicEntityBuilder* configure(const nlohmann::json* object_json, const engine::component::TileInfo* tile_info);

    /// @brief 针对瓦片 (瓦片层，相比“阳光岛”新增的功能)
    BasicEntityBuilder* configure(int index, const engine::component::TileInfo* tile_info);

    virtual BasicEntityBuilder* build();    ///< @brief 构建实体
    entt::entity getEntityID();             ///< @brief 获取实体ID（返回）

protected:
    void reset();       ///< @brief 重置生成器状态，每次configure的时候先调用

    void buildBase();
    void buildSprite();
    void buildTransform();
    void buildAnimation();
    void buildAudio();

    // --- 代理函数，让子类能获取到LevelLoader的私有方法 ---
    template<typename T>
    std::optional<T> getTileProperty(const nlohmann::json& tile_json, std::string_view property_name);
};

} // namespace engine::loader


