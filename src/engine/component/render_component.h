#pragma once
#include "engine/utils/math.h"

namespace engine::component {

/**
 * @brief 渲染组件, 包含图层ID和深度，
 * 颜色调整参数（调整后 = 原始颜色 * 调整颜色）
 */
struct RenderComponent {
    static constexpr int MAIN_LAYER{10};    ///< @brief 主图层ID，默认为10
    
    int layer{};        ///< @brief 图层ID，数字越小越先绘制 
    float depth{};      ///< @brief 在同一图层内的深度，数字越小越先绘制
                        /*  (可用于实现y-sort排序，也可设定其它渲染顺序逻辑) */   
    engine::utils::FColor color_{engine::utils::FColor::white()}; ///< @brief 颜色调整参数

    /**
     * @brief 构造函数
     * @param layer 图层ID，数字越小越先绘制(默认MAIN_LAYER)
     * @param depth 在同一图层内的深度，数字越小越先绘制 (默认0.0f)
     * @param color 颜色调整参数 (调整后 = 原始颜色 * 调整颜色) (默认白色)
     */
    explicit RenderComponent(int layer = MAIN_LAYER, float depth = 0.0f, 
        engine::utils::FColor color = engine::utils::FColor::white()) 
        : layer(layer), depth(depth), color_(color) {}

    // 重载比较运算符，用于排序
    bool operator<(const RenderComponent& other) const {
        if (layer == other.layer) {     // 如果图层相同，则比较深度
            return depth < other.depth;
        }
        return layer < other.layer;    // 如果图层不同，则比较图层ID
    }
};

}   // namespace engine::component