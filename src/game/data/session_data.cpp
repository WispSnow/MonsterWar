#include "session_data.h"
#include <fstream>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <entt/core/hashed_string.hpp>

namespace game::data {

bool SessionData::loadDefaultData(std::string_view path) {

    if (!std::filesystem::exists(path)) {
        spdlog::error("Session data 文件未找到: {}", path);
        return false;
    }
    clear();
    std::filesystem::path file_path = path;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        spdlog::error("无法打开Session data文件: {}", path);
        return false;
    }

    nlohmann::json json;
    file >> json;
    file.close();

    try {
        // 关卡基本信息：当前关卡、积分、是否通关
        level_number_ = json["level"].get<int>();
        point_ = json["point"].get<int>();
        level_clear_ = json["level_clear"].get<bool>();
        // 角色数据：角色名id、职业id、角色名、职业、等级、稀有度
        for (const auto& [name, data] : json["unit"].items()) {
            entt::id_type name_id = entt::hashed_string(name.c_str());
            std::string class_str = data["class"].get<std::string>();
            entt::id_type class_id = entt::hashed_string(class_str.c_str());
            int level = data["level"].get<int>();
            int rarity = data["rarity"].get<int>();
            unit_map_[name_id] = {name_id, class_id, name, class_str, level, rarity};
        }
    } catch (const std::exception& e) {
        spdlog::error("加载Session data失败: {}", e.what());
        return false;
    }
    // map数据加载完毕后，再将角色数据指针插入到unit_data_list_中
    mapUnitDataList();
    return true;
}

bool SessionData::loadFromFile(std::string_view path) {
    return loadDefaultData(path);
}

bool SessionData::saveToFile(std::string_view path) {
    std::filesystem::path file_path = path;
    // 如果父目录不存在，则自动创建
    if (!file_path.parent_path().empty() && !std::filesystem::exists(file_path.parent_path())) {
        try {
            std::filesystem::create_directories(file_path.parent_path());
        } catch (const std::exception& e) {
            spdlog::error("无法创建目录 {}: {}", file_path.parent_path().string(), e.what());
            return false;
        }
    }
    std::ofstream file(file_path);
    if (!file.is_open()) {
        spdlog::error("无法打开存档文件: {}", path);
        return false;
    }

    nlohmann::json json;
    // 关卡基本信息：当前关卡、积分、是否通关
    json["level"] = level_number_;
    json["point"] = point_;
    json["level_clear"] = level_clear_;
    // 角色数据：角色名id、职业id、角色名、职业、等级、稀有度
    for (const auto& [id, data] : unit_map_) {
        std::string name = data.name_;
        json["unit"][name]["class"] = data.class_;
        json["unit"][name]["level"] = data.level_;
        json["unit"][name]["rarity"] = data.rarity_;
    }
    file << json.dump(4);
    file.close();
    spdlog::info("存档文件已保存: {}", path);
    return true;
}

void SessionData::mapUnitDataList() {
    unit_data_list_.clear();
    unit_data_list_.reserve(unit_map_.size());
    for (auto& [id, data] : unit_map_) {
        unit_data_list_.push_back(&data);
    }
}

void SessionData::addUnit(std::string_view name, std::string_view class_str, int level, int rarity) {
    entt::id_type name_id = entt::hashed_string(name.data());
    entt::id_type class_id = entt::hashed_string(class_str.data());
    // 创建角色数据，并插入到unit_map_中
    unit_map_.emplace(name_id, 
                      UnitData{name_id, class_id, std::string(name), std::string(class_str), level, rarity});
    // 将角色数据指针插入到unit_data_list_中
    unit_data_list_.push_back(&unit_map_[name_id]);
}

void SessionData::removeUnit(entt::id_type name_id) {
    if (auto it = unit_map_.find(name_id); it != unit_map_.end()) {
        // 先从unit_data_list_中删除该角色数据指针
        unit_data_list_.erase(std::remove(unit_data_list_.begin(), 
                                          unit_data_list_.end(), 
                                          &unit_map_[name_id]), 
                                          unit_data_list_.end());
        // 再从unit_map_中删除该角色数据
        unit_map_.erase(it);
    } else {
        spdlog::error("未找到该角色: {}", name_id);
    }
}

void SessionData::addUnitLevel(entt::id_type name_id, int add_level) {
    if (auto it = unit_map_.find(name_id); it != unit_map_.end()) {
        it->second.level_ += add_level;
    } else {
        spdlog::error("未找到该角色: {}", name_id);
    }
}

void SessionData::addUnitRarity(entt::id_type name_id, int add_rarity) {
    if (auto it = unit_map_.find(name_id); it != unit_map_.end()) {
        it->second.rarity_ += add_rarity;
    } else {
        spdlog::error("未找到该角色: {}", name_id);
    }
}

void SessionData::clearUnits() {
    unit_map_.clear();
    unit_data_list_.clear();
}

void SessionData::clear() {
    level_number_ = 1;
    unit_map_.clear();
    unit_data_list_.clear();
}

}    // namespace game::data