#include "basic_entity_builder.h"
#include "level_loader.h"
#include "../core/context.h"
#include "../component/tilelayer_component.h"
#include "../component/name_component.h"
#include "../component/sprite_component.h"
#include "../component/transform_component.h"
#include "../resource/resource_manager.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

namespace engine::loader {

BasicEntityBuilder::BasicEntityBuilder(engine::loader::LevelLoader& level_loader, engine::core::Context& context, entt::registry& registry)
    : level_loader_(level_loader), context_(context), registry_(registry) {}

BasicEntityBuilder::~BasicEntityBuilder() = default;

void BasicEntityBuilder::reset() {
    object_json_ = nullptr;
    tile_info_ = nullptr;
    index_ = -1;
    entity_id_ = entt::null;
    position_ = glm::vec2(0.0f);
    dst_size_ = glm::vec2(0.0f);
    src_size_ = glm::vec2(0.0f);
}

BasicEntityBuilder* BasicEntityBuilder::configure(const nlohmann::json* object_json) {
    reset();
    if (!object_json) {
        spdlog::error("配置生成器时，object_json 不能为空");
        return nullptr;
    }
    object_json_ = object_json;
    spdlog::trace("针对自定义形状配置生成器完成");
    return this;
}

BasicEntityBuilder* BasicEntityBuilder::configure(const nlohmann::json* object_json, const engine::component::TileInfo* tile_info) {
    reset();
    if (!object_json || !tile_info) {
        spdlog::error("配置生成器时，object_json 和 tile_info 不能为空");
        return nullptr;
    }

    object_json_ = object_json;
    tile_info_ = tile_info;
    spdlog::trace("针对多图片集合的瓦片配置生成器完成");
    return this;
}

BasicEntityBuilder* BasicEntityBuilder::configure(int index, const engine::component::TileInfo* tile_info) {
    reset();
    if (!tile_info) {
        spdlog::error("配置生成器时，tile_info 不能为空");
        return nullptr;
    }
    index_ = index;
    tile_info_ = tile_info;
    spdlog::trace("针对瓦片配置生成器完成");
    return this;
}

BasicEntityBuilder* BasicEntityBuilder::build() {
    if (!object_json_ && !tile_info_) {
        spdlog::error("object_json 和 tile_info 都为空，无法进行构建");
        return this;
    }

    // 按顺序构建各个组件
    buildBase();
    buildSprite();
    buildTransform();
    buildAnimation();
    buildAudio();
    return this;
}

entt::entity BasicEntityBuilder::getEntityID() {
    return entity_id_;
}

// --- 构建组件 ---
void BasicEntityBuilder::buildBase() {
    spdlog::trace("构建基础组件");
    // 创建一个实体并添加NameComponent组件
    entity_id_ = registry_.create();
    if (object_json_ && object_json_->contains("name")) {
        std::string name = object_json_->value("name", "");
        entt::id_type name_id = entt::hashed_string(name.c_str());
        registry_.emplace<engine::component::NameComponent>(entity_id_, name_id, name);
        spdlog::trace("添加 NameComponent 组件，name: {}", object_json_->value("name", ""));
    }
}

void BasicEntityBuilder::buildSprite() {
    spdlog::trace("构建Sprite组件");
    // 如果是自定义形状对象，则不需要SpriteComponent
    if (!tile_info_) return;
    // 创建Sprite时候确保纹理加载
    auto& resource_manager = context_.getResourceManager();
    resource_manager.loadTexture(tile_info_->sprite_.texture_id_, tile_info_->sprite_.texture_path_);
    registry_.emplace<engine::component::SpriteComponent>(entity_id_, tile_info_->sprite_);
}

void BasicEntityBuilder::buildTransform() {
    spdlog::trace("构建Transform组件");
    glm::vec2 scale = glm::vec2(1.0f);
    float rotation = 0.0f;
    
    // 对象层实体，位置、尺寸和旋转信息从 object_json_ 中获取
    if (object_json_) {
        position_ = glm::vec2(object_json_->value("x", 0.0f), object_json_->value("y", 0.0f));
        dst_size_ = glm::vec2(object_json_->value("width", 0.0f), object_json_->value("height", 0.0f));
        position_ = glm::vec2(position_.x, position_.y - dst_size_.y);  // 图片对象的position需要进行调整(左下角到左上角)
        rotation = object_json_->value("rotation", 0.0f);
        // 如果是图片对象，需要调整缩放
        if (tile_info_) {
            src_size_ = glm::vec2(tile_info_->sprite_.src_rect_.size.x, tile_info_->sprite_.src_rect_.size.y);
            scale = dst_size_ / src_size_;
        }
    }

    // 瓦片层实体，通过index (Tiled瓦片层data数据的索引) 计算位置
    if (index_ >= 0) {
        auto map_size = level_loader_.getMapSize();
        auto tile_size = level_loader_.getTileSize();
        position_ = glm::vec2((index_ % map_size.x) * tile_size.x, 
                             (index_ / map_size.x) * tile_size.y);
    }

    // 添加 TransformComponent
    registry_.emplace<engine::component::TransformComponent>(entity_id_, position_, scale, rotation);
}

void BasicEntityBuilder::buildAnimation() {
    spdlog::trace("构建Animation组件");
    // 如果存在动画，其信息已经解析并保存在tile_info_中
    if (tile_info_ && tile_info_->animation_) {
        // 创建动画map
        std::unordered_map<entt::id_type, engine::component::Animation> animations;
        auto animation_id = entt::hashed_string("tile");    // 图块动画名称默认为"tile"
        animations.emplace(animation_id, std::move(tile_info_->animation_.value()));
        // 通过动画map创建AnimationComponent，并添加
        registry_.emplace<engine::component::AnimationComponent>(entity_id_, std::move(animations), animation_id);
    }
}

void BasicEntityBuilder::buildAudio() {
    spdlog::trace("构建Audio组件");
    // 当前项目并未使用，未来可约定自定义属性并解析
}

// --- 代理函数，让子类能获取到LevelLoader的私有方法 ---
template<typename T>
std::optional<T> BasicEntityBuilder::getTileProperty(const nlohmann::json& tile_json, std::string_view property_name) {
    return level_loader_.getTileProperty<T>(tile_json, property_name);
}

} // namespace engine::loader


