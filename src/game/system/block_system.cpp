#include "block_system.h"
#include "../component/blocker_component.h"
#include "../component/enemy_component.h"
#include "../component/blocked_by_component.h"
#include "../defs/tags.h"
#include "../defs/constants.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/velocity_component.h"
#include "../../engine/utils/events.h"
#include "../../engine/utils/math.h"
#include <entt/entity/view.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::system {

void BlockSystem::update(entt::registry& registry, entt::dispatcher& dispatcher) {
    spdlog::trace("BlockSystem::update");
    // --- 检查阻挡者是否依然有效 ---
    auto view_blocked_by = registry.view<game::component::BlockedByComponent>();   
    for (auto blocked_by_entity : view_blocked_by) {
        auto& blocked_by_component = view_blocked_by.get<game::component::BlockedByComponent>(blocked_by_entity);
        // 如果BlockedBy指向的实体无效(例如死亡)，移除被阻挡组件，并发送播放动画“walk”事件
        if (!registry.valid(blocked_by_component.entity_)) {
            registry.remove<game::component::BlockedByComponent>(blocked_by_entity);
            dispatcher.enqueue(engine::utils::PlayAnimationEvent{blocked_by_entity, "walk"_hs, true});
            spdlog::info("阻挡者: ID: {}, 无效, 移除阻挡者组件", entt::to_integral(blocked_by_entity));
        }
    }

    // --- 判断是否需要添加阻挡者组件 ---
    // 获取所有阻挡者
    auto view_blocker = registry.view<game::component::BlockerComponent, engine::component::TransformComponent>();
    // 获取所有敌人，使用 entt::exclude 排除“包含指定组件的实体”（已经存在阻挡者组件的敌人不需要再添加）
    auto view_enemy = registry.view<game::component::EnemyComponent, 
        engine::component::TransformComponent, 
        engine::component::VelocityComponent>(entt::exclude<game::component::BlockedByComponent>);
    // 遍历所有敌人
    for (auto enemy_entity : view_enemy) {
        const auto& enemy_transform = view_enemy.get<engine::component::TransformComponent>(enemy_entity);
        auto& enemy_velocity = view_enemy.get<engine::component::VelocityComponent>(enemy_entity);
        // 每个敌人遍历所有阻挡者，检查是否被阻挡
        for (auto blocker_entity : view_blocker) {
            const auto& blocker_transform = view_blocker.get<engine::component::TransformComponent>(blocker_entity);
            auto& blocker_blocker = view_blocker.get<game::component::BlockerComponent>(blocker_entity);
            // 如果被阻挡（检查敌人和阻挡者之间的距离是否小于阻挡半径）
            if (engine::utils::distanceSquared(enemy_transform.position_, blocker_transform.position_) < 
                    game::defs::BLOCK_RADIUS * game::defs::BLOCK_RADIUS) {
                // 检查阻挡者是否还能阻挡
                if (blocker_blocker.current_count_ >= blocker_blocker.max_count_) {
                    continue;   // 如果不能阻挡，则跳过
                }
                blocker_blocker.current_count_++;                   // 增加阻挡数量
                enemy_velocity.velocity_ = glm::vec2(0.0f, 0.0f);   // 设置敌人速度为0
                // 给敌人添加被阻挡组件
                registry.emplace<game::component::BlockedByComponent>(enemy_entity, blocker_entity);
                spdlog::info("敌人: ID: {}, 被阻挡, 阻挡者: ID: {}", entt::to_integral(enemy_entity), entt::to_integral(blocker_entity));
                
                // 播放动画“attack” （临时测试用，未来会将攻击逻辑放在其他系统里）
                dispatcher.enqueue(engine::utils::PlayAnimationEvent{enemy_entity, "attack"_hs, true});
            }
        }
    }
}

}   // namespace game::system