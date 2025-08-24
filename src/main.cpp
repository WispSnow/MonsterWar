#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

void ImGuiInit(SDL_Window *window, SDL_Renderer *renderer) {
    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 初始化 ImGui 的 SDL3 和 SDL_Renderer3 后端
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void ImGuiLoop(SDL_Renderer *renderer) {
    // ImGui: 开始新帧
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // 显示一个Demo窗口 （UI声明与逻辑交互）
    ImGui::ShowDemoWindow();

    // ImGui: 渲染
    ImGui::Render();    // 生成绘图数据
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);  // 执行渲染
}

void ImGuiShutdown() {
    // ImGui: 清理工作
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

int main(int, char**) {

    // SDL初始化
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        spdlog::error("SDL_Init Error: {}", SDL_GetError());
        return 1;
    }
    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("Hello World!", 1280, 800, 0);
    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // 1. ImGui 初始化
    ImGuiInit(window, renderer);

    // 渲染循环
    while (true) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                break;
            }
            // 2. ImGui: 处理 ImGui 事件
            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        // 清屏
        SDL_RenderClear(renderer);

        // 3. 一轮循环内，ImGui 需要做的操作（逻辑+渲染）
        ImGuiLoop(renderer);

        // 更新屏幕
        SDL_RenderPresent(renderer);
    }

    // 4. ImGui 清理
    ImGuiShutdown();

    // SDL清理并退出
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}