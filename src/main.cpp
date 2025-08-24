#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

void ImGuiOptionalSettings() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // 设置 ImGui 主题
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // 设置缩放
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());     // 与系统缩放一致
    // float main_scale = 1.0f;     // 或者直接设置更加稳定
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // 固定样式缩放比例。
    style.FontScaleDpi = main_scale;        // 设置初始字体缩放比例。

    // 设置透明度
    float window_alpha = 0.5f;

    // 修改各个UI元素的透明度
    style.Colors[ImGuiCol_WindowBg].w = window_alpha;
    style.Colors[ImGuiCol_PopupBg].w = window_alpha;

    // 为了正确显示中文，我们需要加载支持中文的字体。
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "assets/fonts/VonwaonBitmap-16px.ttf",  // 字体文件路径
        16.0f,                                  // 字体大小
        nullptr,                                // 字体配置参数
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon() // 字符范围
    );
    if (!font)
    {
        // 如果字体加载失败，回退到默认字体，但中文将无法显示。
        io.Fonts->AddFontDefault();
        spdlog::warn("警告：无法加载中文字体，中文字符将无法正确显示。");
    }
}

void ImGuiInit(SDL_Window *window, SDL_Renderer *renderer) {
    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 可选配置
    ImGuiOptionalSettings();

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