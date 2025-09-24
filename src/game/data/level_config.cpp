#include "level_config.h"
#include <filesystem>   
#include <fstream>
#include <utility>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <entt/core/hashed_string.hpp>

namespace game::data {

bool LevelConfig::loadFromFile(std::string_view level_json_path) {
    auto path = std::filesystem::path(level_json_path);
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("无法打开关卡配置文件: {}", level_json_path);
        return false;
    }
    nlohmann::ordered_json json;
    file >> json;
    file.close();
    try {
        if (!json.is_array()) {
            spdlog::error("关卡配置文件不是数组");
            return false;
        }
        int level_number = 1;   // 关卡编号，从1开始
        // 数组中每个json对象对应每一关
        for (const auto& data : json) {
            game::data::LevelData level_data;
            level_data.level_number_ = level_number;
            level_data.enemy_level_ = data["enemy_level"].get<int>();
            level_data.name_ = data["name"].get<std::string>();
            level_data.map_path_ = data["map_path"].get<std::string>();
            level_data.prep_time_ = data["prep_time"].get<float>();
            level_data.enemy_rarity_ = data["enemy_rarity"].get<int>();

            int total_enemy_count = 0;
            if (data.contains("waves") && data["waves"].is_array()) {
                // 读取波次数据，每一波都有自己的波次间隔和生成间隔
                for (const auto& wave : data["waves"]) {
                    game::data::Wave wave_data;
                    wave_data.next_wave_interval_ = wave["next_wave_interval"].get<float>();
                    wave_data.spawn_interval_ = wave["spawn_interval"].get<float>();
                    // 每个波次中，保存了敌人类型和对应的数量
                    for (const auto& [enemy_type, count] : wave["enemy_types"].items()) {
                        entt::id_type type_id = entt::hashed_string(enemy_type.c_str());
                        auto count_int = count.get<int>();
                        total_enemy_count += count_int;     // 更新总敌人数量
                        wave_data.enemy_types_.push_back(std::pair<entt::id_type, int>(type_id, count_int));
                    }
                    level_data.waves_data_.waves_.push(std::move(wave_data)); // 将波次数据加入关卡数据
                }
            }
            level_data.total_enemy_count_ = total_enemy_count;
            spdlog::info("关卡 {} 总敌人数量: {}", level_data.level_number_, total_enemy_count);

            // 设定第一波的准备时间（来自json中的"prep_time"）
            level_data.waves_data_.next_wave_count_down_ = level_data.prep_time_;

            // 解析完毕，将关卡数据加入关卡数据数组
            level_data_.push_back(std::move(level_data));

            // 更新关卡编号准备下一关
            level_number++;
        }
        spdlog::info("关卡配置加载完成");
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("解析波次文件失败: {}", e.what());
        return false;
    }
    return true;
}

}   // namespace game::data