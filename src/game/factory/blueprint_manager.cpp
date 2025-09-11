#include "blueprint_manager.h"
#include "../../engine/resource/resource_manager.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

namespace game::factory {

BlueprintManager::BlueprintManager(engine::resource::ResourceManager& resource_manager)
    : resource_manager_(resource_manager) {}

bool BlueprintManager::loadEnemyClassBlueprints(std::string_view enemy_json_path) {
    auto path = std::filesystem::path(enemy_json_path);
    std::ifstream file(path);
    nlohmann::json json;
    file >> json;
    file.close();
    // --- 解析蓝图 ---
    try {
        for (auto& [class_name, data_json] : json.items()) {
            entt::id_type class_id = entt::hashed_string(class_name.c_str());
            // 解析 Stats
            data::StatsBlueprint stats = parseStats(data_json);
            // 解析 Sprite
            data::SpriteBlueprint sprite = parseSprite(data_json);
            // 解析 Animation
            std::unordered_map<entt::id_type, data::AnimationBlueprint> animations = parseAnimationsMap(data_json);
            // 解析Sound
            data::SoundBlueprint sounds = parseSound(data_json);
            // 解析Enemy数据
            data::EnemyBlueprint enemy = parseEnemy(data_json);
            // 解析DisplayInfo
            data::DisplayInfoBlueprint display_info = parseDisplayInfo(data_json);
            // 解析完毕，组合蓝图并插入容器
            enemy_class_blueprints_.emplace(class_id, data::EnemyClassBlueprint{class_id, 
                class_name, 
                std::move(stats),
                std::move(enemy),
                std::move(sounds),
                std::move(sprite),
                std::move(display_info),
                std::move(animations)});
        }
    } catch (const std::exception& e) {
        spdlog::error("加载敌人单位数据时出错: {}", e.what());
        return false;
    }
    return true;
}

const data::EnemyClassBlueprint& BlueprintManager::getEnemyClassBlueprint(entt::id_type id) const {
    if (auto it = enemy_class_blueprints_.find(id); it != enemy_class_blueprints_.end()) {
        return it->second;
    }
    spdlog::error("未找到对应 id 的 EnemyClassBlueprint: {}", id);
    return enemy_class_blueprints_.begin()->second;
}

// --- 拆分步骤的私有解析函数 ---

data::StatsBlueprint BlueprintManager::parseStats(const nlohmann::json& json) {
    return data::StatsBlueprint{json["hp"].get<float>(), 
        json["atk"].get<float>(), 
        json["def"].get<float>(), 
        json["range"].get<float>(),
        json["atk_interval"].get<float>()};
}

data::SpriteBlueprint BlueprintManager::parseSprite(const nlohmann::json& json) {
    auto width = json["width"].get<float>();
    auto height = json["height"].get<float>();
    auto path_str = json["sprite_sheet"].get<std::string>();
    auto path_id = entt::hashed_string(path_str.c_str());
    // 可选部分：源矩形的起点默认值为 0,0，渲染目标大小默认值为 width,height
    // （如果指定，起点为 x,y，渲染目标大小为 size_x,size_y）
    return data::SpriteBlueprint{path_id, 
        path_str, 
        engine::utils::Rect{glm::vec2(json.value("x", 0), json.value("y", 0)), glm::vec2(width, height)}, 
        glm::vec2(json.value("size_x", width), json.value("size_y", height)),
        glm::vec2(json.value("offset_x", 0), json.value("offset_y", 0)),
        json.value("face_right", true)
    };
}

std::unordered_map<entt::id_type, data::AnimationBlueprint> BlueprintManager::parseAnimationsMap(const nlohmann::json& json) {
    std::unordered_map<entt::id_type, data::AnimationBlueprint> animations; // 先准备好容器
    for (auto& [anim_name, anim_data] : json["animation"].items()) {
        // 每轮循环解析一个具体的动画 （动画名，动画数据）
        auto anim_name_id = entt::hashed_string(anim_name.c_str());
        std::vector<int> frames = anim_data["frames"].get<std::vector<int>>();  // 动画帧信息数组可直接获取
        // 创建单个动画蓝图，并插入容器
        data::AnimationBlueprint animation{anim_data.value("duration", 100.0f), 
            anim_data.value("row", 0), 
            std::move(frames)
        };
        animations.emplace(anim_name_id, animation);
    }
    return animations;
}

data::SoundBlueprint BlueprintManager::parseSound(const nlohmann::json& json) {
    data::SoundBlueprint sounds;
    if (json.contains("sounds")) {  // 如果包含音效
        // 遍历所有音效键值对
        for (auto& [sound_key, sound_value] : json["sounds"].items()) {
            // 先把 sound_value 看成是音效路径并通过资源管理器加载
            std::string sound_path = sound_value.get<std::string>();
            entt::id_type sound_id = entt::hashed_string(sound_path.c_str());
            resource_manager_.loadSound(sound_id, sound_path);
            // 将音效键值对转换为音效ID并插入到声音蓝图中
            sounds.sounds_.emplace(entt::hashed_string(sound_key.c_str()), sound_id);
        }
    }
    return sounds;
}

data::EnemyBlueprint BlueprintManager::parseEnemy(const nlohmann::json& json) {
    // 敌人组件蓝图只包含“是否远程”和“移动速度”
    return data::EnemyBlueprint{json["ranged"].get<bool>(), json["speed"].get<float>()};
}

data::DisplayInfoBlueprint BlueprintManager::parseDisplayInfo(const nlohmann::json& json) {
    // 显示信息蓝图只包含“名称”和“描述”
    return data::DisplayInfoBlueprint{json.value("name", ""), json.value("description", "")};
}

}   // namespace game::factory