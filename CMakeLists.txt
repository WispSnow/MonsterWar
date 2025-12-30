# ============================================
# 项目主配置文件
# ============================================
cmake_minimum_required(VERSION 3.10.0)
project(MonsterWar VERSION 0.1.0 LANGUAGES C CXX)

# ============================================
# 基础配置
# ============================================

# 指定搜索路径（用于find_package在编译期查找库）
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)

# 指定目标名称
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# 依赖库默认链接类型：ON = 动态链接(.dll/.so/.dylib)，OFF = 静态链接(.lib/.a)
# 注意：可以在Dependencies.cmake中为每个库单独指定
option(BUILD_SHARED_LIBS "依赖库默认编译为动态库" OFF)

# ============================================
# 引入模块化配置
# ============================================

# 编译器设置（C++标准、编译选项等）
include(cmake/CompilerSettings.cmake)

# 运行时路径配置（RPATH）
include(cmake/RuntimePath.cmake)

# 依赖管理
include(cmake/Dependencies.cmake)

# ImGui配置
include(cmake/ImGui.cmake)

# 构建辅助函数
include(cmake/BuildHelpers.cmake)

# 项目信息打印
include(cmake/ProjectInfo.cmake)

# ============================================
# 设置项目依赖
# ============================================

# 调用依赖配置函数（定义在Dependencies.cmake中）
setup_project_dependencies()

# ============================================
# 项目源文件
# ============================================

set(SOURCES
    src/main.cpp
    # Engine - Audio
    src/engine/audio/audio_player.cpp
    # Engine - Core
    src/engine/core/game_app.cpp
    src/engine/core/time.cpp
    src/engine/core/config.cpp
    src/engine/core/context.cpp
    src/engine/core/game_state.cpp
    # Engine - Resource
    src/engine/resource/resource_manager.cpp
    src/engine/resource/texture_manager.cpp
    src/engine/resource/audio_manager.cpp
    src/engine/resource/font_manager.cpp
    # Engine - Render
    src/engine/render/renderer.cpp
    src/engine/render/camera.cpp
    src/engine/render/text_renderer.cpp
    # Engine - Input
    src/engine/input/input_manager.cpp
    # Engine - Loader
    src/engine/loader/level_loader.cpp
    src/engine/loader/basic_entity_builder.cpp
    # Engine - Scene
    src/engine/scene/scene.cpp
    src/engine/scene/scene_manager.cpp
    # Engine - System
    src/engine/system/audio_system.cpp
    src/engine/system/animation_system.cpp
    src/engine/system/render_system.cpp
    src/engine/system/movement_system.cpp
    src/engine/system/ysort_system.cpp
    # Engine - UI
    src/engine/ui/ui_manager.cpp
    src/engine/ui/ui_element.cpp
    src/engine/ui/ui_interactive.cpp
    src/engine/ui/ui_panel.cpp
    src/engine/ui/ui_image.cpp
    src/engine/ui/ui_label.cpp
    src/engine/ui/ui_button.cpp
    src/engine/ui/state/ui_normal_state.cpp
    src/engine/ui/state/ui_pressed_state.cpp
    src/engine/ui/state/ui_hover_state.cpp
    # Game - Data
    src/game/data/session_data.cpp
    src/game/data/ui_config.cpp
    src/game/data/level_config.cpp
    # Game - Factory
    src/game/factory/blueprint_manager.cpp
    src/game/factory/entity_factory.cpp
    # Game - Loader
    src/game/loader/entity_builder_mw.cpp
    # Game - Scene
    src/game/scene/game_scene.cpp
    src/game/scene/title_scene.cpp
    src/game/scene/end_scene.cpp
    src/game/scene/level_clear_scene.cpp
    # Game - Spawner
    src/game/spawner/enemy_spawner.cpp
    # Game - System
    src/game/system/followpath_system.cpp
    src/game/system/remove_dead_system.cpp
    src/game/system/block_system.cpp
    src/game/system/attack_starter_system.cpp
    src/game/system/timer_system.cpp
    src/game/system/orientation_system.cpp
    src/game/system/set_target_system.cpp
    src/game/system/animation_state_system.cpp
    src/game/system/animation_event_system.cpp
    src/game/system/combat_resolve_system.cpp
    src/game/system/projectile_system.cpp
    src/game/system/effect_system.cpp
    src/game/system/health_bar_system.cpp
    src/game/system/game_rule_system.cpp
    src/game/system/place_unit_system.cpp
    src/game/system/render_range_system.cpp
    src/game/system/debug_ui_system.cpp
    src/game/system/selection_system.cpp
    src/game/system/skill_system.cpp
    # Game - UI
    src/game/ui/units_portrait_ui.cpp
)

# Windows平台添加资源文件
if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

# ============================================
# 可执行文件配置
# ============================================

# 创建可执行文件
add_executable(${TARGET} ${SOURCES} ${IMGUI_SOURCES})

# 设置包含路径，让包含可以从 src/ 开始
target_include_directories(${TARGET} PRIVATE src)

# 链接所有依赖库
target_link_libraries(${TARGET}
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    glm::glm
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    EnTT::EnTT
)

# ============================================
# 应用配置
# ============================================

# 设置编译选项（定义在CompilerSettings.cmake中）
setup_compiler_options(${TARGET})

# 配置资源文件复制（定义在BuildHelpers.cmake中）
setup_asset_copy(${TARGET})

# 配置Windows DLL复制（定义在BuildHelpers.cmake中）
setup_windows_dll_copy(${TARGET})

# ============================================
# 打印配置信息
# ============================================

# 打印项目配置完成信息（定义在ProjectInfo.cmake中）
print_project_info(${TARGET})
