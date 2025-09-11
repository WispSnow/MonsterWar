#pragma once
#include <glm/vec2.hpp>
#include <string_view>
#include <random>

namespace engine::utils {

/**
 * @brief 自定义矩形结构体，包含位置和大小。
 */
struct Rect
{
    glm::vec2 position{};
    glm::vec2 size{};
    
    Rect() = default;
    Rect(glm::vec2 position, glm::vec2 size) : position(position), size(size) {}
    Rect(float x, float y, float width, float height) : position(x, y), size(width, height) {}
};

/**
 * @brief 自定义颜色结构体。
 */
struct FColor
{
    float r{};
    float g{};
    float b{};
    float a{};
};

/**
 * @brief 解析十六进制颜色字符串（如 "#RRGGBB" 或 "#RRGGBBAA"）为 FColor
 * @param hex_color 颜色字符串（支持 "#RRGGBB" 或 "#RRGGBBAA" 格式）
 * @return FColor 结构体，若解析失败则返回全 0
 */
 constexpr FColor parseHexColor(std::string_view hex_color) {
    // 16进制符号（字符）转为10进制整数的工具函数
    auto hexToInt = [](char c) -> int {
        if ('0' <= c && c <= '9') return c - '0';           // 0-9: 针对'0'的偏移
        if ('a' <= c && c <= 'f') return 10 + (c - 'a');    // a-f: 针对'a'的偏移 + 10
        if ('A' <= c && c <= 'F') return 10 + (c - 'A');    // A-F: 针对'A'的偏移 + 10
        return 0;
    };

    // 检查有效性 (第一个字符必须是#，总长度必须为7位或者9位)
    if (hex_color.empty() || hex_color[0] != '#') return {0.0f, 0.0f, 0.0f, 0.0f};
    size_t len = hex_color.length();
    if (len != 7 && len != 9) return {0.0f, 0.0f, 0.0f, 0.0f}; // 只支持 #RRGGBB 或 #RRGGBBAA

    // 解析rgb颜色分量（每个颜色2位，高位*16 + 低位），范围 0-255
    int r = hexToInt(hex_color[1]) * 16 + hexToInt(hex_color[2]);
    int g = hexToInt(hex_color[3]) * 16 + hexToInt(hex_color[4]);
    int b = hexToInt(hex_color[5]) * 16 + hexToInt(hex_color[6]);

    // 解析alpha分量（没有该数据则使用默认值255不透明）
    int a = 255;
    if (len == 9) {
        a = hexToInt(hex_color[7]) * 16 + hexToInt(hex_color[8]);
    }

    // 返回归一化到0.0-1.0范围的颜色值
    return {
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        static_cast<float>(a) / 255.0f
    };
}

/**
 * @brief 生成指定范围内的随机整数 [min, max]
 * @param min 最小值（包含）
 * @param max 最大值（包含）
 * @return 随机整数
 */
 inline int randomInt(int min, int max) {
    // static thread_local 表示该变量在每个线程中各自独立，互不影响，避免多线程下的竞争条件
    static thread_local std::mt19937 generator{std::random_device{}()};
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

/**
 * @brief 根据等级和稀有度修改属性
 * @param base 基础属性
 * @param level 等级
 * @param rarity 稀有度
 * @return 修改后的属性
 */
inline float statModify(float base, int level = 1, int rarity = 1) {
    return base * (0.95f + 0.05f * level) * (0.9f + 0.1f * rarity);
    // NOTE: 未来可改成数据驱动方便调整
}

} // namespace engine::utils