#include "level_loader.h"
#include "../scene/scene.h"
#include "../core/context.h"
#include "../resource/resource_manager.h"
#include "../component/tilelayer_component.h"
#include "../component/name_component.h"
#include "../component/sprite_component.h"
#include "../component/transform_component.h"
#include "../component/parallax_component.h"
#include "../render/renderer.h"
#include "../utils/math.h"
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <SDL3/SDL_rect.h>
#include <entt/entity/registry.hpp>
#include <entt/core/hashed_string.hpp>

namespace engine::loader {

LevelLoader::~LevelLoader() = default;

void LevelLoader::setEntityBuilder(std::unique_ptr<BasicEntityBuilder> builder) {
    entity_builder_ = std::move(builder);
}

bool LevelLoader::loadLevel(std::string_view level_path, engine::scene::Scene* scene) {
    if (!scene) {
        spdlog::error("场景指针为空");
        return false;
    }
    scene_ = scene;

    if (!entity_builder_) {
        spdlog::info("设置默认的实体生成器");
        entity_builder_ = std::make_unique<BasicEntityBuilder>(*this, scene->getContext(), scene->getRegistry());
    }

    // 1. 加载 JSON 文件
    auto path = std::filesystem::path(level_path);
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("无法打开关卡文件: {}", level_path);
        return false;
    }

    // 2. 解析 JSON 数据
    nlohmann::json json_data;
    try {
        file >> json_data;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("解析 JSON 数据失败: {}", e.what());
        return false;
    }

    // 3. 获取基本地图信息 (名称、地图尺寸、瓦片尺寸)
    map_path_ = level_path;
    map_size_ = glm::ivec2(json_data.value("width", 0), json_data.value("height", 0));
    tile_size_ = glm::ivec2(json_data.value("tilewidth", 0), json_data.value("tileheight", 0));

    // 4. 加载 tileset 数据
    if (json_data.contains("tilesets") && json_data["tilesets"].is_array()) {
        for (const auto& tileset_json : json_data["tilesets"]) {
            if (!tileset_json.contains("source") || !tileset_json["source"].is_string() ||
                !tileset_json.contains("firstgid") || !tileset_json["firstgid"].is_number_integer()) {
                spdlog::error("tilesets 对象中缺少有效 'source' 或 'firstgid' 字段。");
                continue;
            }
            auto tileset_path = resolvePath(tileset_json["source"].get<std::string>(), map_path_);  // 支持隐式转换，可以省略.get<T>()方法，
            auto first_gid = tileset_json["firstgid"];
            loadTileset(tileset_path, first_gid);
        }
    }

    // 5. 加载图层数据
    if (!json_data.contains("layers") || !json_data["layers"].is_array()) {       // 地图文件中必须有 layers 数组
        spdlog::error("地图文件 '{}' 中缺少或无效的 'layers' 数组。", level_path);
        return false;
    }
    for (const auto& layer_json : json_data["layers"]) {
        // 获取各图层对象中的类型（type）字段
        std::string layer_type = layer_json.value("type", "none");
        if (!layer_json.value("visible", true)) {
            spdlog::info("图层 '{}' 不可见，跳过加载。", layer_json.value("name", "Unnamed"));
            continue;
        }

        // 根据图层类型决定加载方法
        if (layer_type == "imagelayer") {       
            loadImageLayer(layer_json);
        } else if (layer_type == "tilelayer") {
            loadTileLayer(layer_json);
        } else if (layer_type == "objectgroup") {
            loadObjectLayer(layer_json);
        } else {
            spdlog::warn("不支持的图层类型: {}", layer_type);
        }
    }

    spdlog::info("关卡加载完成: {}", level_path);
    return true;
}

void LevelLoader::loadImageLayer(const nlohmann::json& layer_json) {
    // 获取纹理相对路径 （会自动处理'\/\'符号）
    std::string image_path = layer_json.value("image", "");     // json.value()返回的是一个临时对象，需要赋值才能保存，
                                                                 // 不能用std::string_view
    if (image_path.empty()) {
        spdlog::error("图层 '{}' 缺少 'image' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }

    // 创建精灵 (在获取纹理大小时会确保纹理加载)
    auto texture_path = resolvePath(image_path, map_path_); 
    auto& resource_manager = scene_->getContext().getResourceManager();
    auto texture_size = resource_manager.getTextureSize(entt::hashed_string(texture_path.c_str()), texture_path);
    auto sprite = engine::component::Sprite(texture_path, engine::utils::Rect{glm::vec2(0.0f), texture_size});

    // 获取图层偏移量（json中没有则代表未设置，给默认值即可）
    const glm::vec2 offset = glm::vec2(layer_json.value("offsetx", 0.0f), layer_json.value("offsety", 0.0f));
    
    // 获取视差因子及重复标志
    const glm::vec2 scroll_factor = glm::vec2(layer_json.value("parallaxx", 1.0f), layer_json.value("parallaxy", 1.0f));
    const glm::bvec2 repeat = glm::bvec2(layer_json.value("repeatx", false), layer_json.value("repeaty", false));
    
    // 获取图层名称
    std::string layer_name = layer_json.value("name", "Unnamed");
    entt::id_type name_id = entt::hashed_string(layer_name.c_str());
    
    /*  可用类似方法获取其它各种属性，这里我们暂时用不上 */
    
    // 创建图层实体
    auto& registry = scene_->getRegistry();
    auto entity = registry.create();

    // 添加组件
    registry.emplace<engine::component::NameComponent>(entity, name_id, layer_name);
    registry.emplace<engine::component::TransformComponent>(entity, offset);
    registry.emplace<engine::component::ParallaxComponent>(entity, scroll_factor, repeat);
    registry.emplace<engine::component::SpriteComponent>(entity, sprite);
    /* 实体与组件创建完毕后即由registry自动管理，不需要“添加到场景”的步骤 */

    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadTileLayer(const nlohmann::json& layer_json) {
    if (!layer_json.contains("data") || !layer_json["data"].is_array()) {
        spdlog::error("图层 '{}' 缺少 'data' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }

    // 获取图层名称
    std::string layer_name = layer_json.value("name", "Unnamed");
    entt::id_type name_id = entt::hashed_string(layer_name.c_str());

    // 创建图层实体
    auto& registry = scene_->getRegistry();
    auto layer_entity = registry.create();
    registry.emplace<engine::component::NameComponent>(layer_entity, name_id, layer_name);

    // 准备瓦片实体vector (瓦片数量 = 地图宽度 * 地图高度)
    std::vector<entt::entity> tiles;
    tiles.reserve(map_size_.x * map_size_.y);

    // 获取图层数据 (瓦片 ID 列表)
    const auto& data = layer_json["data"];

    size_t index = 0;   // data数据的索引，它决定图块在地图中的位置
    // --- 每一个瓦片都是一个独立的entity ---
    for (const int gid : data) {
        if (gid == 0) {
            index++;
            continue;
        }
        auto tile_info = getTileInfoByGid(gid);
        if (!tile_info) {
            spdlog::error("瓦片 ID 为 {} 的瓦片未找到图块集。", gid);
            index++;
            continue;
        }
        // 使用生成器创建瓦片实体
        auto tile_entity = entity_builder_->configure(index, &tile_info.value())->build()->getEntityID();
        // 添加到vector中
        tiles.push_back(tile_entity);
        index++;
    }

    // 最后将瓦片层组件添加到图层实体中
    registry.emplace<engine::component::TileLayerComponent>(layer_entity, tile_size_, map_size_, tiles);

    spdlog::info("加载图层: '{}' 完成", layer_name);
}

void LevelLoader::loadObjectLayer(const nlohmann::json& layer_json) {
    if (!layer_json.contains("objects") || !layer_json["objects"].is_array()) {
        spdlog::error("对象图层 '{}' 缺少 'objects' 属性。", layer_json.value("name", "Unnamed"));
        return;
    }
    // 获取对象数据
    const auto& objects = layer_json["objects"];
    // 遍历对象数据
    for (const auto& object : objects) {
        // 获取对象gid
        auto gid = object.value("gid", 0);
        if (gid == 0) {     // 如果gid为0 (即不存在)，则代表自己绘制的形状
            // 配置生成器，并调用build，针对自定义形状
            entity_builder_->configure(&object)->build();

        } else {        // 如果gid存在，则按照图片解析流程
            // 配置生成器，针对图片对象
            auto tile_info = getTileInfoByGid(gid);
            if (!tile_info) {
                spdlog::warn("对象图层 '{}' 中的对象缺少有效的 'gid' 或瓦片信息。", layer_json.value("name", "Unnamed"));
                continue;
            }
            // 配置生成器，并调用build，针对图片对象
            entity_builder_->configure(&object, &tile_info.value())->build();
        }
    }
}

void LevelLoader::loadTileset(std::string_view tileset_path, int first_gid) {
    auto path = std::filesystem::path(tileset_path);
    std::ifstream tileset_file(path);
    if (!tileset_file.is_open()) {
        spdlog::error("无法打开 Tileset 文件: {}", tileset_path);
        return;
    }

    nlohmann::json ts_json;
    try {
        tileset_file >> ts_json;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::error("解析 Tileset JSON 文件 '{}' 失败: {} (at byte {})", tileset_path, e.what(), e.byte);
        return;
    }
    ts_json["file_path"] = tileset_path;    // 将文件路径存储到json中，后续解析图片路径时需要
    tileset_data_[first_gid] = std::move(ts_json);
    spdlog::info("Tileset 文件 '{}' 加载完成，firstgid: {}", tileset_path, first_gid);
}

std::optional<engine::utils::Rect> LevelLoader::getColliderRect(const nlohmann::json& tile_json) {
    if (!tile_json.contains("objectgroup")) return std::nullopt;
    auto& objectgroup = tile_json["objectgroup"];
    if (!objectgroup.contains("objects")) return std::nullopt;
    auto& objects = objectgroup["objects"];
    for (const auto& object : objects) {    // 一个图片只支持一个碰撞器。如果有多个，则返回第一个不为空的
        auto rect = engine::utils::Rect(glm::vec2(object.value("x", 0.0f), object.value("y", 0.0f)), 
                                        glm::vec2(object.value("width", 0.0f), object.value("height", 0.0f)));
        if (rect.size.x > 0 && rect.size.y > 0) {
            return rect;
        }
    }
    return std::nullopt;    // 如果没找到碰撞器，则返回空
}

engine::utils::Rect LevelLoader::getTextureRect(const nlohmann::json& tileset_json, int local_id) {
    auto columns = tileset_json.value("columns", 1);
    auto tile_width = tileset_json.value("tilewidth", 0);
    auto tile_height = tileset_json.value("tileheight", 0);
    auto coordinate_x = local_id % columns;
    auto coordinate_y = local_id / columns;
    return engine::utils::Rect{glm::vec2(coordinate_x * tile_width, coordinate_y * tile_height), 
                               glm::vec2(tile_width, tile_height)};
}

engine::component::TileType LevelLoader::getTileType(const nlohmann::json& tile_json) {
    if (tile_json.contains("properties")) {
        auto& properties = tile_json["properties"];
        for (auto& property : properties) {
            if (property.contains("name") && property["name"] == "solid") {
                auto is_solid = property.value("value", false);
                return is_solid ? engine::component::TileType::SOLID : engine::component::TileType::NORMAL;
            }
            else if (property.contains("name") && property["name"] == "hazard") {
                auto is_hazard = property.value("value", false);
                return is_hazard ? engine::component::TileType::HAZARD : engine::component::TileType::NORMAL;
            }
            // TODO: 可以在这里添加更多的自定义属性处理逻辑
        }
    }
    return engine::component::TileType::NORMAL;
}

engine::component::TileType LevelLoader::getTileTypeById(const nlohmann::json& tileset_json, int local_id) {
    if (tileset_json.contains("tiles")) {
        auto& tiles = tileset_json["tiles"];
        for (auto& tile : tiles) {
            if (tile.contains("id") && tile["id"] == local_id) {
                return getTileType(tile);
            }
        }
    }
    return engine::component::TileType::NORMAL;
}

std::optional<engine::component::TileInfo> LevelLoader::getTileInfoByGid(int gid) {
    if (gid == 0) {
        return std::nullopt;
    }

    // upper_bound：查找tileset_data_中键大于 gid 的第一个元素，返回迭代器
    auto tileset_it = tileset_data_.upper_bound(gid);
    if (tileset_it == tileset_data_.begin()) {
        spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
        return std::nullopt;
    } 
    --tileset_it;  // 前移一个位置，这样就得到不大于gid的最近一个元素（我们需要的）

    const auto& tileset = tileset_it->second;
    auto local_id = gid - tileset_it->first;        // 计算瓦片在图块集中的局部ID
    std::string file_path = tileset.value("file_path", "");       // 获取图块集文件路径
    if (file_path.empty()) {
        spdlog::error("Tileset 文件 '{}' 缺少 'file_path' 属性。", tileset_it->first);
        return std::nullopt;
    }

    engine::component::TileInfo tile_info;  // 初始化瓦片信息
    // 图块集分为两种情况，用一个标志进行记录区分
    bool is_single_image = false;
    if (tileset.contains("image")) {    // 这是单一图片的情况
        // 获取必要信息
        auto texture_rect = getTextureRect(tileset, local_id);
        auto image_path = tileset["image"].get<std::string>();
        // 计算纹理绝对路径
        auto texture_path = resolvePath(image_path, file_path);
        // 创建精灵
        tile_info.sprite_ = engine::component::Sprite(texture_path, texture_rect);
        tile_info.type_ = getTileTypeById(tileset, local_id);   // 获取瓦片类型（只有瓦片id，还没找具体瓦片json）
        is_single_image = true;
    } 
    // --- 考虑多图片的情况 ---
    if (!is_single_image && !tileset.contains("tiles")) {   // 没有tiles字段的话不符合数据格式要求，直接返回空的瓦片信息
        spdlog::error("Tileset 文件 '{}' 缺少 'tiles' 属性。", tileset_it->first);
        return std::nullopt;
    }
    // 遍历tiles数组，根据id查找对应的瓦片
    const auto& tiles_json = tileset["tiles"];
    for (const auto& tile_json : tiles_json) {
        auto tile_id = tile_json.value("id", 0);
        if (tile_id == local_id) {   // 找到对应的瓦片，进行后续操作
            // 如果是多图片，需要先补充精灵和类型信息
            if (!is_single_image) {
                if (!tile_json.contains("image")) {   // 没有image字段的话不符合数据格式要求，直接返回空的瓦片信息
                    spdlog::error("Tileset 文件 '{}' 中瓦片 {} 缺少 'image' 属性。", tileset_it->first, tile_id);
                    return std::nullopt;
                }
                // --- 接下来根据必要信息创建并返回 TileInfo ---
                // 获取图片路径
                auto texture_path = resolvePath(tile_json["image"].get<std::string>(), file_path);
                // 先确认图片尺寸
                auto image_width = tile_json.value("imagewidth", 0);
                auto image_height = tile_json.value("imageheight", 0);
                // 从json中获取源矩形信息
                engine::utils::Rect texture_rect = {      // tiled中源矩形信息只有设置了才会有值，没有就是默认值
                    glm::vec2(tile_json.value("x", 0.0f), tile_json.value("y", 0.0f)),
                    glm::vec2(tile_json.value("width", image_width), tile_json.value("height", image_height))
                };
                tile_info.sprite_ = engine::component::Sprite(texture_path, texture_rect);
                scene_->getContext().getResourceManager().loadTexture(entt::hashed_string(texture_path.c_str()), texture_path);  // 确保纹理被加载
                tile_info.type_ = getTileType(tile_json);    // 获取瓦片类型（已经有具体瓦片json了）
            }
            // 补充动画信息 （瓦片动画为animation字段，且必须为数组，目前只考虑单一图片情况）
            if (tile_json.contains("animation") && is_single_image && tile_json["animation"].is_array()) {
                std::vector<engine::component::AnimationFrame> animation_frames;
                auto& animation = tile_json["animation"];
                for (auto& frame : animation) {
                    // 每个瓦片动画帧json有两个信息：tileid 和 duration
                    float duration_ms = frame.value("duration", 100.0f);
                    int id = frame.value("tileid", 0);
                    auto frame_rect = getTextureRect(tileset, id);  // 根据id获取纹理源矩形
                    // 源矩形 + 时长，组成一个动画帧
                    auto animation_frame = engine::component::AnimationFrame(frame_rect, duration_ms);
                    animation_frames.push_back(animation_frame);
                }
                tile_info.animation_ = engine::component::Animation(std::move(animation_frames));
            }
            // 补充属性信息
            if (tile_json.contains("properties")) {
                tile_info.properties_ = tile_json["properties"];
            }
        }
    }

    return tile_info;
}

std::string LevelLoader::resolvePath(std::string_view relative_path, std::string_view file_path) {
    try {   
        // 获取地图文件的父目录（相对于可执行文件） "assets/maps/level1.tmj" -> "assets/maps"
        auto map_dir = std::filesystem::path(file_path).parent_path();
        // 合并路径（相对于可执行文件）并返回。 /* std::filesystem::canonical：解析路径中的当前目录（.）和上级目录（..）导航符，
                                          /*  得到一个干净的路径 */
        auto final_path = std::filesystem::canonical(map_dir / relative_path);
        return final_path.string();
    } catch (const std::exception& e) {
        spdlog::error("解析路径失败: {}", e.what());
        return std::string(relative_path);
    }
}

} // namespace engine::loader


