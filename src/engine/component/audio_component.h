#pragma once
#include <entt/entity/fwd.hpp>
#include <unordered_map>

namespace engine::component {

/**
 * @brief 音频组件，包含音效集合。
 */
struct AudioComponent {
    std::unordered_map<entt::id_type, entt::id_type> sounds_;   ///< @brief 音效集合，名称(哈希) -> 音效ID
};

} // namespace engine::component