#pragma once
#include "../data/entity_blueprint.h"
#include <string_view>
#include <unordered_map>
#include <entt/entity/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

namespace engine::resource {
    class ResourceManager;
}

namespace game::factory {

/**
 * @brief 蓝图管理器，用于存储、管理所有蓝图
 * 
 * 它从json数据中加载蓝图并保存到容器，并和获取蓝图的功能。蓝图信息将由实体工厂使用。
 */
class BlueprintManager {
    friend class EntityFactory;

private:
    engine::resource::ResourceManager& resource_manager_;

    std::unordered_map<entt::id_type, data::EnemyClassBlueprint> enemy_class_blueprints_;   ///< @brief 敌人类型蓝图
    // TODO: 未来添加其他蓝图容器

public:
    BlueprintManager(engine::resource::ResourceManager& resource_manager);

    [[nodiscard]] bool loadEnemyClassBlueprints(std::string_view enemy_json_path);      ///< @brief 加载敌人类型蓝图, 返回是否成功
    // TODO: 未来添加其他蓝图加载函数

    const data::EnemyClassBlueprint& getEnemyClassBlueprint(entt::id_type id) const;    ///< @brief 获取指定ID的敌人类型蓝图
    // TODO: 未来添加其他蓝图获取函数

private:
    // --- 分别针对各个子蓝图进行json解析，并创建(返回)对应的蓝图结构体 ---
    data::StatsBlueprint parseStats(const nlohmann::json& json);
    data::SpriteBlueprint parseSprite(const nlohmann::json& json);
    std::unordered_map<entt::id_type, data::AnimationBlueprint> parseAnimationsMap(const nlohmann::json& json);
    data::SoundBlueprint parseSound(const nlohmann::json& json);
    data::EnemyBlueprint parseEnemy(const nlohmann::json& json);
    data::DisplayInfoBlueprint parseDisplayInfo(const nlohmann::json& json);
};

}   // namespace game::factory