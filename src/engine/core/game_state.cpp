#include "game_state.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::core {

GameState::GameState(SDL_Window* window, SDL_Renderer* renderer, State initial_state)
    : window_(window), renderer_(renderer), current_state_(initial_state){
    if (window_ == nullptr || renderer_ == nullptr) {
        spdlog::error("窗口或渲染器为空");
        throw std::runtime_error("窗口或渲染器不能为空");
    }
    if (!syncLogicalPresentationState()) {
        spdlog::warn("无法读取初始逻辑分辨率，将在首次设置时更新缓存。");
    }
    spdlog::trace("游戏状态初始化完成");
}

void GameState::setState(State new_state) {
    if (current_state_ != new_state) {
        spdlog::debug("游戏状态改变");
        current_state_ = std::move(new_state);
    } else {
        spdlog::debug("尝试设置相同的游戏状态，跳过");
    }
}

glm::vec2 GameState::getWindowSize() const
{
    int width, height;
    // SDL3获取窗口大小的方法
    SDL_GetWindowSize(window_, &width, &height);
    return glm::vec2(width, height);
}

void GameState::setWindowSize(const glm::vec2& window_size)
{
    SDL_SetWindowSize(window_, static_cast<int>(window_size.x), static_cast<int>(window_size.y));
}

glm::vec2 GameState::getLogicalSize() const
{
    int width = 0;
    int height = 0;
    SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
    if (!SDL_GetRenderLogicalPresentation(renderer_, &width, &height, &mode)) {
        spdlog::error("获取逻辑分辨率失败：{}", SDL_GetError());
        return glm::vec2(logical_width_, logical_height_);
    }
    if (mode == SDL_LOGICAL_PRESENTATION_DISABLED &&
        logical_width_ > 0 && logical_height_ > 0) {
        return glm::vec2(logical_width_, logical_height_);
    }
    return glm::vec2(width, height);
}


void GameState::setLogicalSize(const glm::vec2& logical_size)
{
    const int logical_width = static_cast<int>(logical_size.x);
    const int logical_height = static_cast<int>(logical_size.y);
    if (!SDL_SetRenderLogicalPresentation(renderer_,
                                          logical_width,
                                          logical_height,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        spdlog::error("设置逻辑分辨率失败：{}", SDL_GetError());
        return;
    }
    logical_width_ = logical_width;
    logical_height_ = logical_height;
    logical_mode_ = SDL_LOGICAL_PRESENTATION_LETTERBOX;
    logical_presentation_disabled_ = false;
    spdlog::trace("逻辑分辨率设置为: {}x{}", logical_size.x, logical_size.y);
}

bool GameState::disableLogicalPresentation()
{
    int width = 0;
    int height = 0;
    SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
    if (!SDL_GetRenderLogicalPresentation(renderer_, &width, &height, &mode)) {
        spdlog::error("读取当前逻辑分辨率失败：{}", SDL_GetError());
        return false;
    }
    if (mode == SDL_LOGICAL_PRESENTATION_DISABLED) {
        logical_presentation_disabled_ = true;
        return true;
    }

    logical_width_ = width;
    logical_height_ = height;
    logical_mode_ = mode;
    if (!SDL_SetRenderLogicalPresentation(renderer_,
                                          logical_width_,
                                          logical_height_,
                                          SDL_LOGICAL_PRESENTATION_DISABLED)) {
        spdlog::error("关闭逻辑分辨率失败：{}", SDL_GetError());
        return false;
    }
    logical_presentation_disabled_ = true;
    return true;
}

bool GameState::enableLogicalPresentation()
{
    if (!logical_presentation_disabled_) {
        return true;
    }
    if (logical_width_ <= 0 || logical_height_ <= 0 ||
        logical_mode_ == SDL_LOGICAL_PRESENTATION_DISABLED) {
        spdlog::error("没有可恢复的逻辑分辨率配置。");
        return false;
    }
    if (!SDL_SetRenderLogicalPresentation(renderer_,
                                          logical_width_,
                                          logical_height_,
                                          logical_mode_)) {
        spdlog::error("恢复逻辑分辨率失败：{}", SDL_GetError());
        return false;
    }
    logical_presentation_disabled_ = false;
    return true;
}

bool GameState::syncLogicalPresentationState()
{
    int width = 0;
    int height = 0;
    SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
    if (!SDL_GetRenderLogicalPresentation(renderer_, &width, &height, &mode)) {
        spdlog::error("同步逻辑分辨率状态失败：{}", SDL_GetError());
        return false;
    }
    if (mode != SDL_LOGICAL_PRESENTATION_DISABLED) {
        logical_width_ = width;
        logical_height_ = height;
        logical_mode_ = mode;
        logical_presentation_disabled_ = false;
    }
    return true;
}


} // namespace engine::core
