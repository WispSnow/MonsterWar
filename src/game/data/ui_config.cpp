#include "ui_config.h"
#include "engine/render/image.h"
#include <fstream>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <entt/core/hashed_string.hpp>


namespace game::data {

UIConfig::~UIConfig() = default;

bool UIConfig::loadFromFile(std::string_view path) {
    std::filesystem::path file_path(path);
    std::ifstream file(file_path);
    nlohmann::json json;
    file >> json;

    try {
        loadIcon(json["icon"]);
        loadPortrait(json["portrait"]);
        loadPortraitFrame(json["portrait_frame"]);
        loadLayout(json["layout"]);
    } catch (const std::exception& e) {
        spdlog::error("载入 UI config 失败: {}", e.what());
        return false;
    }
    return true;
}

void UIConfig::loadIcon(nlohmann::json& json) {
    for (auto& [key, value] : json.items()) {
        entt::id_type id = entt::hashed_string(key.c_str());
        auto texture_path = value["sprite_sheet"].get<std::string>();
        engine::utils::Rect src_rect = {static_cast<float>(value["x"]), 
            static_cast<float>(value["y"]), 
            static_cast<float>(value["width"]), 
            static_cast<float>(value["height"])};
        icon_map_[id] = engine::render::Image(texture_path, src_rect, false);
    }
}

void UIConfig::loadPortrait(nlohmann::json& json) {
    for (auto& [key, value] : json.items()) {
        entt::id_type id = entt::hashed_string(key.c_str());
        auto texture_path = value["sprite_sheet"].get<std::string>();
        engine::utils::Rect src_rect = {static_cast<float>(value["x"]), 
            static_cast<float>(value["y"]), 
            static_cast<float>(value["width"]), 
            static_cast<float>(value["height"])};
        portrait_map_[id] = engine::render::Image(texture_path, src_rect, false);
    }
}

void UIConfig::loadPortraitFrame(nlohmann::json& json) {
    for (auto& [key, value] : json.items()) {
        auto texture_path = value["sprite_sheet"].get<std::string>();
        int level = value["level"].get<int>();
        engine::utils::Rect src_rect = {static_cast<float>(value["x"]), 
            static_cast<float>(value["y"]), 
            static_cast<float>(value["width"]), 
            static_cast<float>(value["height"])};
        portrait_frame_map_[level] = engine::render::Image(texture_path, src_rect, false);
    }
}

void UIConfig::loadLayout(nlohmann::json& json) {
    unit_panel_padding_ = json["unit_panel"]["padding"].get<float>();
    unit_panel_frame_size_ = {json["unit_panel"]["frame_size"]["width"].get<float>(), 
                              json["unit_panel"]["frame_size"]["height"].get<float>()};
    unit_panel_font_size_ = json["unit_panel"]["font_size"].get<int>();
    unit_panel_font_path_ = json["unit_panel"]["font_path"].get<std::string>();
    unit_panel_font_offset_ = {json["unit_panel"]["font_offset"]["x"].get<float>(), 
                               json["unit_panel"]["font_offset"]["y"].get<float>()};
}

engine::render::Image& UIConfig::getIcon(entt::id_type id) {
    if (auto it = icon_map_.find(id); it != icon_map_.end()) {
        return it->second;
    } else {
        spdlog::error("Icon 未找到: {}", id);
        return icon_map_.begin()->second;
    }
}

engine::render::Image& UIConfig::getPortrait(entt::id_type id) {
    if (auto it = portrait_map_.find(id); it != portrait_map_.end()) {
        return it->second;
    } else {
        spdlog::error("Portrait 未找到: {}", id);
        return portrait_map_.begin()->second;
    }
}

engine::render::Image& UIConfig::getPortraitFrame(int rarity) {
    if (auto it = portrait_frame_map_.find(rarity); it != portrait_frame_map_.end()) {
        return it->second;
    } else {
        spdlog::error("Portrait Frame 未找到: {}", rarity);
        return portrait_frame_map_.begin()->second;
    }
}

} // namespace game::data