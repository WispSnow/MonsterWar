#include "projectile_system.h"
#include "game/component/projectile_component.h"
#include "game/defs/tags.h"
#include "game/defs/events.h"
#include "game/factory/entity_factory.h"
#include "engine/component/transform_component.h"
#include "engine/utils/events.h"
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <spdlog/spdlog.h>

using namespace entt::literals;

namespace game::system {

ProjectileSystem::ProjectileSystem(entt::registry& registry, entt::dispatcher& dispatcher, game::factory::EntityFactory& entity_factory)
    : registry_(registry), dispatcher_(dispatcher), entity_factory_(entity_factory) {
    dispatcher_.sink<game::defs::EmitProjectileEvent>().connect<&ProjectileSystem::onEmitProjectileEvent>(this);
}

ProjectileSystem::~ProjectileSystem() {
    dispatcher_.disconnect(this);
}

void ProjectileSystem::update(float delta_time) {
    // 获取所有投射物
    auto view = registry_.view<game::component::ProjectileComponent, engine::component::TransformComponent>();
    for (auto entity : view) {
        auto& projectile = registry_.get<game::component::ProjectileComponent>(entity);
        auto& transform = registry_.get<engine::component::TransformComponent>(entity);
        // 更新飞行时间
        projectile.current_flight_time_ += delta_time;
        // 如果飞行时间超过总飞行时间，则命中目标（发送攻击事件以及播放音效）并销毁
        if (projectile.current_flight_time_ >= projectile.total_flight_time_) {
            dispatcher_.enqueue(game::defs::AttackEvent{entity, projectile.target_, projectile.damage_});
            dispatcher_.enqueue(engine::utils::PlaySoundEvent{entity, "hit"_hs});
            registry_.emplace<game::defs::DeadTag>(entity);
            continue;
        }
        // 计算飞行进度 (t 从 0 到 1)
        float t = projectile.current_flight_time_ / projectile.total_flight_time_;
        t = glm::clamp(t, 0.0f, 1.0f); // 确保 t 在 [0, 1] 区间

        // 1. 计算水平位置 (线性插值)
        glm::vec2 horizontal_pos = glm::mix(projectile.start_position_, projectile.target_position_, t);

        // 2. 计算垂直方向的弧线偏移
        // 使用 sin 函数可以轻松创建弧线: sin(0)=0, sin(PI/2)=1, sin(PI)=0
        float arc_offset = glm::sin(t * glm::pi<float>()) * projectile.arc_height_;

        // 3. 合成最终位置
        transform.position_ = horizontal_pos;
        transform.position_.y -= arc_offset; // Y轴向下为正，所以减去偏移使其向上拱起

        // 4. 根据上一帧的位置计算朝向，并更新TransformComponent的旋转参数
        auto direction = transform.position_ - projectile.previous_position_;
        transform.rotation_ = glm::atan(direction.y, direction.x) * 180.0f / glm::pi<float>();

        // 5. 更新上一帧的位置
        projectile.previous_position_ = transform.position_;
    }
}

void ProjectileSystem::onEmitProjectileEvent(const game::defs::EmitProjectileEvent& event) {
    spdlog::info("发射投射物: {}", event.id_);
    entity_factory_.createProjectile(event.id_, 
        event.start_position_,
        event.target_position_, 
        event.target_, 
        event.damage_);
}

}   // namespace game::system