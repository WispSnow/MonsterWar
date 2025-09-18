#pragma once
#include "../data/entity_blueprint.h"
#include <entt/entity/fwd.hpp>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace game::factory {

class BlueprintManager;
/**
 * @brief 实体工厂，用于创建不同类型的实体
 * 
 * 实体工厂通过蓝图管理器获取蓝图数据，并创建不同类型的实体。
 */
class EntityFactory {
private:
    entt::registry& registry_;
    BlueprintManager& blueprint_manager_;

public:
    /// @brief 实体工厂构造函数, 需要传入注册表和蓝图管理器。通过蓝图数据创建不同实体
    EntityFactory(entt::registry& registry, BlueprintManager& blueprint_manager);

    /**
     * @brief 创建玩家单位
     * @param class_id 职业ID
     * @param position 位置
     * @param level 等级
     * @param rarity 稀有度
     * @return 玩家单位实体
     */
    entt::entity createPlayerUnit(entt::id_type class_id, const glm::vec2& position, int level = 1, int rarity = 1);

    /**
     * @brief 创建敌人单位
     * @param class_id 敌人类型ID
     * @param position 位置
     * @param target_waypoint_id 目标路径点ID
     * @param level 等级
     * @param rarity 稀有度
     * @return 敌人单位实体
     */
    entt::entity createEnemyUnit(entt::id_type class_id, const glm::vec2& position, int target_waypoint_id, int level = 1, int rarity = 1);

    /**
     * @brief 创建投射物
     * @param id 投射物ID
     * @param start_position 起始位置
     * @param target_position 目标位置
     * @param target 目标实体
     * @param damage 伤害
     * @return 投射物实体
     */
    entt::entity createProjectile(entt::id_type id, const glm::vec2& start_position, const glm::vec2& target_position, entt::entity target, float damage);
    
    /**
     * @brief 创建敌人死亡特效
     * @note 敌人死亡特效直接从敌人蓝图中获取，对应的动画名称必须为“damage”。
     * @param class_id 敌人ID
     * @param position 位置
     * @param is_flipped 是否翻转
     * @return 敌人死亡特效实体
     */
    entt::entity createEnemyDeadEffect(entt::id_type class_id, const glm::vec2& position, const bool is_flipped = false);
    // TODO: 未来添加其他实体的创建函数

private:
    // --- 组件创建函数 ---
    void addTransformComponent(entt::entity entity, const glm::vec2& position, const glm::vec2& scale = glm::vec2(1.0f), float rotation = 0.0f);
    void addSpriteComponent(entt::entity entity, const data::SpriteBlueprint& sprite, const bool is_flipped = false);
    void addAnimationComponent(entt::entity entity,         ///< @brief 正常动画组件添加（多个动画）
        const std::unordered_map<entt::id_type, data::AnimationBlueprint>& animation_blueprints, 
        const data::SpriteBlueprint& sprite_blueprint,
        entt::id_type default_animation_id);
    void addOneAnimationComponent(entt::entity entity,      ///< @brief 单个动画组件添加（组件中只包含一个动画），用于创建特效
        const data::AnimationBlueprint& animation_blueprint, 
        const data::SpriteBlueprint& sprite_blueprint,
        entt::id_type animation_id,
        bool loop = false);
    void addStatsComponent(entt::entity entity, const data::StatsBlueprint& stats, int level = 1, int rarity = 1);
    void addPlayerComponent(entt::entity entity, const data::PlayerBlueprint& player, int rarity);
    void addEnemyComponent(entt::entity entity, const data::EnemyBlueprint& enemy, int target_waypoint_id);
    void addAudioComponent(entt::entity entity, const data::SoundBlueprint& sounds);
    void addProjectileIDComponent(entt::entity entity, entt::id_type id);
    // TODO: 未来添加其他组件创建函数
};

}   // namespace game::factory