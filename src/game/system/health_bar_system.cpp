#include "health_bar_system.h"
#include "../component/stats_component.h"
#include "../../engine/component/transform_component.h"
#include "../defs/tags.h"
#include "../defs/constants.h"
#include "../../engine/render/renderer.h"
#include "../../engine/render/camera.h"
#include "../../engine/utils/math.h"
#include <entt/entity/registry.hpp>

namespace game::system {

void HealthBarSystem::update(entt::registry& registry, engine::render::Renderer& renderer, engine::render::Camera& camera) {
    // 只有受伤的实体才显示血量标签
    auto view = registry.view<engine::component::TransformComponent,
        game::component::StatsComponent,
        game::defs::HasHealthBarTag,
        game::defs::InjuredTag>();

    for (auto entity : view) {
        const auto [transform, stats] = view.get<engine::component::TransformComponent, game::component::StatsComponent>(entity);

        auto size = game::defs::HEALTH_BAR_SIZE;
        // 血量条位置 = 角色位置 + 偏移量
        auto position = transform.position_ + glm::vec2(-size.x / 2.0f, game::defs::HEALTH_BAR_OFFSET_Y);
        
        // 根据血量百分比确定颜色
        auto health_percent = static_cast<float>(stats.hp_) / static_cast<float>(stats.max_hp_);
        engine::utils::FColor color;
        if (health_percent > 0.7f) {
            color = engine::utils::FColor::green();
        } else if (health_percent > 0.3f) {
            color = engine::utils::FColor::orange();
        } else {
            color = engine::utils::FColor::red();
        }

        // 执行绘制(先画边框，再画血量)
        renderer.drawRect(camera, position, size, color);
        size.x = size.x * health_percent;
        renderer.drawFilledRect(camera, position, size, color);
    }
}

} // namespace game::system