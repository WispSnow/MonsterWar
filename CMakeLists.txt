# 标题
cmake_minimum_required(VERSION 3.10.0)
project(MonsterWar VERSION 0.1.0 LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# 指定搜索路径（用于find_package在编译期查找库）
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)

# 指定目标名称
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# ============================================
# 运行时动态库搜索路径配置
# ============================================
# 解决运行时找不到动态库的问题

# 1. 设置RPATH（macOS和Linux）
# RPATH是嵌入到可执行文件中的运行时库搜索路径
if(APPLE)
    # macOS使用@executable_path相对路径
    set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/lib;@executable_path/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib;${CMAKE_SOURCE_DIR}/prebuilt/lib")
elseif(UNIX)
    # Linux使用$ORIGIN相对路径
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/lib:$ORIGIN/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib:${CMAKE_SOURCE_DIR}/prebuilt/lib")
endif()

# 构建时使用RPATH，不需要设置LD_LIBRARY_PATH就能运行
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# 2. macOS额外配置
if(APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
endif()

# ============================================
# 项目配置选项
# ============================================

# 依赖库默认链接类型：ON = 动态链接(.dll/.so/.dylib)，OFF = 静态链接(.lib/.a)
# 注意：可以在调用find_or_fetch_dependency时为每个库单独指定
option(BUILD_SHARED_LIBS "依赖库默认编译为动态库" OFF)

# 说明：
# - 依赖库会自动智能选择获取方式：
#   1. 优先使用系统已安装的库（find_package）
#   2. 如果本地external/目录存在源码，则使用本地源码编译
#   3. 否则自动从GitHub在线获取并编译
# - 国内用户可从课程网盘下载依赖库放入external/目录，避免网络问题

# 包含FetchContent模块
include(FetchContent)

# 设置FetchContent使用浅克隆
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ============================================
# 辅助宏：查找或获取依赖（智能浅克隆 + 链接类型控制）
# 用法如下：
#
# 参数说明：
#   DEP_NAME      - 依赖的内部名称（用于FetchContent和add_subdirectory等）
#   PACKAGE_NAME  - 依赖的包名（用于find_package查找本地已安装的包）
#   GIT_REPO      - 依赖的Git仓库地址（用于FetchContent在线获取源码）
#   GIT_TAG       - 依赖的Git分支、标签或commit hash（用于指定获取源码的版本）
#   LOCAL_PATH    - 本地源码路径（如 external/SDL），用于本地源码方式
#   LINK_TYPE     - 链接类型：STATIC(静态) / SHARED(动态) / AUTO(使用全局BUILD_SHARED_LIBS)
# ============================================
macro(find_or_fetch_dependency DEP_NAME PACKAGE_NAME GIT_REPO GIT_TAG LOCAL_PATH LINK_TYPE)
    message(STATUS "正在处理依赖: ${DEP_NAME}")
    
    # 确定该库的链接类型
    if("${LINK_TYPE}" STREQUAL "STATIC")
        set(_LIB_IS_SHARED OFF)
        set(_LINK_TYPE_STR "静态")
    elseif("${LINK_TYPE}" STREQUAL "SHARED")
        set(_LIB_IS_SHARED ON)
        set(_LINK_TYPE_STR "动态")
    else() # AUTO 或其他值，使用全局设置
        set(_LIB_IS_SHARED ${BUILD_SHARED_LIBS})
        if(BUILD_SHARED_LIBS)
            set(_LINK_TYPE_STR "动态(全局)")
        else()
            set(_LINK_TYPE_STR "静态(全局)")
        endif()
    endif()
    
    # 首先尝试查找本地已安装的包
    find_package(${PACKAGE_NAME} QUIET)
    
    if(${PACKAGE_NAME}_FOUND OR ${DEP_NAME}_FOUND)
        message(STATUS "  ✓ 找到本地安装的 ${PACKAGE_NAME}")
        
        # 打印包的路径信息（尝试多种可能的变量）
        if(DEFINED ${PACKAGE_NAME}_DIR)
            message(STATUS "     路径: ${${PACKAGE_NAME}_DIR}")
        elseif(DEFINED ${DEP_NAME}_DIR)
            message(STATUS "     路径: ${${DEP_NAME}_DIR}")
        elseif(DEFINED ${PACKAGE_NAME}_ROOT)
            message(STATUS "     根目录: ${${PACKAGE_NAME}_ROOT}")
        elseif(DEFINED ${DEP_NAME}_ROOT)
            message(STATUS "     根目录: ${${DEP_NAME}_ROOT}")
        elseif(DEFINED ${PACKAGE_NAME}_INCLUDE_DIRS)
            message(STATUS "     头文件: ${${PACKAGE_NAME}_INCLUDE_DIRS}")
        elseif(DEFINED ${DEP_NAME}_INCLUDE_DIRS)
            message(STATUS "     头文件: ${${DEP_NAME}_INCLUDE_DIRS}")
        endif()
    else()
        message(STATUS "  ✗ 未找到本地安装，准备从源码构建 [${_LINK_TYPE_STR}]")
        
        # 配置库特定的编译选项（在add_subdirectory/FetchContent之前）
        # SDL系列库的特定选项
        if("${DEP_NAME}" MATCHES "^SDL" OR "${PACKAGE_NAME}" MATCHES "^SDL")
            if(_LIB_IS_SHARED)
                set(SDL_SHARED ON CACHE BOOL "" FORCE)
                set(SDL_STATIC OFF CACHE BOOL "" FORCE)
            else()
                set(SDL_SHARED OFF CACHE BOOL "" FORCE)
                set(SDL_STATIC ON CACHE BOOL "" FORCE)
            endif()
            # 禁用SDL测试
            set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
            set(SDL_TESTS OFF CACHE BOOL "" FORCE)
            set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
        endif()
        
        # 其他库的通用选项
        set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
        
        # SDL_image特定选项：禁用可能导致构建问题的格式支持
        if("${DEP_NAME}" STREQUAL "SDL3_image")
            # 禁用AVIF格式（需要NASM、Meson等复杂工具链）
            set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SHARED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SAVE OFF CACHE BOOL "" FORCE)
            # 禁用AVIF的依赖库
            set(SDLIMAGE_AVIF_VENDORED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_DAV1D OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AOM OFF CACHE BOOL "" FORCE)
            # 可选：禁用其他可能有构建问题的格式
            # set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)  # JPEG XL
        endif()
        
        # 智能选择：优先本地源码，否则在线获取
        set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
        if(EXISTS ${LOCAL_SOURCE_DIR})
            # 检测到本地源码，使用本地编译
            message(STATUS "  → 使用本地源码: ${LOCAL_SOURCE_DIR}")
            
            # 临时设置BUILD_SHARED_LIBS影响该库
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)

            # add_subdirectory参数说明：
            # 1.源码目录（${LOCAL_SOURCE_DIR}），即依赖库的本地源码路径；
            # 2.二进制输出目录（${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build），即该依赖库的构建输出会放到这里，避免污染主项目的build目录。
            add_subdirectory(${LOCAL_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build)
            
            # 恢复全局设置
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
            
            # 确保命名空间别名存在（兼容性处理）
            # 某些库从源码编译时可能不会自动创建带命名空间的ALIAS target
            # 这里统一创建 PACKAGE_NAME::PACKAGE_NAME 格式的别名
            if(TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${DEP_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${DEP_NAME})
            elseif(TARGET ${PACKAGE_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${PACKAGE_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${PACKAGE_NAME})
            endif()
        else()
            # 本地源码不存在，使用FetchContent在线获取
            message(STATUS "  → 本地源码不存在，在线获取: ${GIT_REPO}")
            
            # 智能检测GIT_TAG类型，决定是否使用浅克隆
            string(LENGTH "${GIT_TAG}" TAG_LENGTH)
            string(REGEX MATCH "^[0-9a-f]+$" IS_HEX "${GIT_TAG}")
            
            # 判断是否为commit hash（40位十六进制字符串）
            if(TAG_LENGTH EQUAL 40 AND IS_HEX)
                message(STATUS "     版本: commit hash [${GIT_TAG}]，完整克隆")
                set(USE_SHALLOW FALSE)
            else()
                message(STATUS "     版本: ${GIT_TAG}，浅克隆")
                set(USE_SHALLOW TRUE)
            endif()
            
            # FetchContent_Declare用于声明一个FetchContent对象，用于在线获取依赖库源码。
            # 参数说明：
            #   DEP_NAME       - 依赖的内部名称（用于FetchContent和add_subdirectory等）
            #   GIT_REPOSITORY - 依赖的Git仓库地址
            #   GIT_TAG        - 依赖的Git分支、标签或commit hash
            #   GIT_SHALLOW    - 是否使用浅克隆（TRUE/FALSE），根据GIT_TAG类型自动判断
            #   GIT_PROGRESS   - 是否显示Git进度（TRUE/FALSE）
            
            # 临时设置BUILD_SHARED_LIBS影响该库
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            
            FetchContent_Declare(
                ${DEP_NAME}
                GIT_REPOSITORY ${GIT_REPO}
                GIT_TAG ${GIT_TAG}
                GIT_SHALLOW ${USE_SHALLOW}
                GIT_PROGRESS TRUE
            )
            FetchContent_MakeAvailable(${DEP_NAME})
            
            # 恢复全局设置
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
        endif()
    endif()
endmacro()

# ============================================
# 依赖配置
# ============================================
# SDL3
find_or_fetch_dependency(
    SDL3
    SDL3
    "https://github.com/libsdl-org/SDL.git"
    "release-3.2.24"
    "external/SDL-release-3.2.24"
    AUTO  # 使用全局BUILD_SHARED_LIBS设置
)

# SDL3_image
# 注意：已自动禁用AVIF格式支持（需要NASM、Meson等复杂工具链）
# 如需启用AVIF，请修改宏中的SDLIMAGE_AVIF选项，并安装所需工具：
#   - NASM: https://www.nasm.us/
#   - Meson: pip install meson ninja
#   - Perl: https://strawberryperl.com/
find_or_fetch_dependency(
    SDL3_image
    SDL3_image
    "https://github.com/libsdl-org/SDL_image.git"
    "release-3.2.4"
    "external/SDL_image-release-3.2.4"
    AUTO  # 使用全局BUILD_SHARED_LIBS设置
)

# SDL3_mixer
find_or_fetch_dependency(
    SDL3_mixer
    SDL3_mixer
    "https://github.com/libsdl-org/SDL_mixer.git"
    "30c1301055a35ee87b8679279b6fc88e10d28fa3"
    "external/SDL_mixer-30c1301"
    AUTO  # 使用全局BUILD_SHARED_LIBS设置
)

# SDL3_ttf
find_or_fetch_dependency(
    SDL3_ttf
    SDL3_ttf
    "https://github.com/libsdl-org/SDL_ttf.git"
    "release-3.2.2"
    "external/SDL_ttf-release-3.2.2"
    AUTO  # 使用全局BUILD_SHARED_LIBS设置
)

# GLM
find_or_fetch_dependency(
    glm
    glm
    "https://github.com/g-truc/glm.git"
    "1.0.1"
    "external/glm-1.0.1"
    STATIC  # GLM通常使用静态链接，动态很可能出错
)

# nlohmann-json (header-only库)
# 注意：FetchContent内部名称用json，但find_package用nlohmann_json
find_or_fetch_dependency(
    json
    nlohmann_json
    "https://github.com/nlohmann/json.git"
    "v3.12.0"
    "external/json-3.12.0"
    STATIC  # header-only库，实际不影响
)

# spdlog
find_or_fetch_dependency(
    spdlog
    spdlog
    "https://github.com/gabime/spdlog.git"
    "v1.15.3"
    "external/spdlog-1.15.3"
    STATIC  # 推荐静态链接，避免运行时依赖
)

# EnTT (header-only库)
find_or_fetch_dependency(
    EnTT
    EnTT
    "https://github.com/skypjack/entt.git"
    "v3.15.0"
    "external/entt-3.15.0"
    STATIC  # header-only库，实际不影响
)

# 设置通用源文件
set(SOURCES
    src/main.cpp
    src/engine/audio/audio_player.cpp
    src/engine/core/game_app.cpp
    src/engine/core/time.cpp
    src/engine/core/config.cpp
    src/engine/core/context.cpp
    src/engine/core/game_state.cpp
    src/engine/resource/resource_manager.cpp
    src/engine/resource/texture_manager.cpp
    src/engine/resource/audio_manager.cpp
    src/engine/resource/font_manager.cpp
    src/engine/render/renderer.cpp
    src/engine/render/camera.cpp
    src/engine/render/text_renderer.cpp
    src/engine/input/input_manager.cpp
    src/engine/loader/level_loader.cpp
    src/engine/loader/basic_entity_builder.cpp
    src/engine/scene/scene.cpp
    src/engine/scene/scene_manager.cpp
    src/engine/system/audio_system.cpp
    src/engine/system/animation_system.cpp
    src/engine/system/render_system.cpp
    src/engine/system/movement_system.cpp
    src/engine/system/ysort_system.cpp
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
    src/game/factory/blueprint_manager.cpp
    src/game/factory/entity_factory.cpp
    src/game/loader/entity_builder_mw.cpp
    src/game/scene/game_scene.cpp
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
)

# --- 添加 ImGui 开始 ---

# 定义 ImGui 源文件变量，方便管理
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    # Backend files
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

# 将 ImGui 的目录和其后端目录添加到头文件搜索路径
include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# --- 添加 ImGui 结束 ---

# 根据平台向"通用源文件"追加平台专属的文件
# if(WIN32)
#     # 如果是 Windows 平台, 将资源文件追加到源文件列表
#     list(APPEND SOURCES resources.rc)
# endif()
# TODO: MacOS 和 Linux 平台

# ============================================
# 可执行文件配置
# ============================================

# 添加可执行文件, 使用配置好的 "SOURCES" 变量
add_executable(${TARGET} ${SOURCES} ${IMGUI_SOURCES})

# 链接库
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
# 编译选项配置
# ============================================
# 注意：必须在add_executable()之后设置，确保目标已定义
# 使用target_compile_options()只影响主目标，不影响依赖库

if(MSVC)
    # Visual Studio: 启用所有警告 + UTF-8编码支持
    target_compile_options(${TARGET} PRIVATE /W4 /utf-8)
    # 不要弹出控制台窗口
    target_link_options(${TARGET} PRIVATE "/SUBSYSTEM:WINDOWS")
elseif(WIN32 AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    # MinGW/Clang on Windows: 设置UTF-8编码
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -finput-charset=utf-8 -fexec-charset=utf-8)
else()
    # Linux/macOS: 标准警告选项
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================
# 编译前配置
# ============================================

# 可执行文件不再生成在根目录，因此需要复制 assets 到 exe 所在目录
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake CONTENT "
# 生成的Assets复制脚本
set(SOURCE_DIR \"${CMAKE_CURRENT_LIST_DIR}/assets\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>/assets\")

# 计算源文件夹大小
file(GLOB_RECURSE SOURCE_FILES \"\${SOURCE_DIR}/*\")
set(SOURCE_SIZE 0)
foreach(FILE IN LISTS SOURCE_FILES)
    file(SIZE \"\${FILE}\" FILE_SIZE)
    math(EXPR SOURCE_SIZE \"\${SOURCE_SIZE} + \${FILE_SIZE}\")
endforeach()

# 计算目标文件夹大小
set(TARGET_SIZE 0)
if(EXISTS \"\${TARGET_DIR}\")
    file(GLOB_RECURSE TARGET_FILES \"\${TARGET_DIR}/*\")
    foreach(FILE IN LISTS TARGET_FILES)
        file(SIZE \"\${FILE}\" FILE_SIZE)
        math(EXPR TARGET_SIZE \"\${TARGET_SIZE} + \${FILE_SIZE}\")
    endforeach()
endif()

# 比较大小，只在不同时复制
if(NOT SOURCE_SIZE EQUAL TARGET_SIZE)
    file(COPY \"\${SOURCE_DIR}/\" DESTINATION \"\${TARGET_DIR}\")
    message(STATUS \"更新资源文件夹 (大小: \${SOURCE_SIZE} 字节)\")
endif()
")

add_custom_command(TARGET ${TARGET} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_assets_$<CONFIG>.cmake
    COMMENT "检查并复制资源文件"
)

# ============================================
# 编译后配置
# ============================================

# Windows不支持RPATH，还需要需要将DLL复制到exe同目录
if(WIN32)
    message(STATUS "配置Windows DLL自动检测和复制...")
    
    # 1. 复制prebuilt目录中的预编译DLL（如果存在）
    if(EXISTS ${CMAKE_SOURCE_DIR}/prebuilt/bin)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/prebuilt/bin $<TARGET_FILE_DIR:${TARGET}>
            COMMENT "复制预编译DLL到可执行文件目录"
        )
    endif()
    
    # 2. 自动检测并复制所有运行时DLL依赖（CMake 3.21+）
    # $<TARGET_RUNTIME_DLLS>会自动检测所有动态库依赖
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        # 使用file(GENERATE)来正确展开生成器表达式
        file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake CONTENT "
# 生成的DLL和PDB复制脚本
set(DLL_LIST \"$<TARGET_RUNTIME_DLLS:${TARGET}>\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET}>\")

if(DLL_LIST)
    # 统计实际需要复制的文件数
    set(COPIED_COUNT 0)
    set(COPIED_FILES \"\")
    
    foreach(DLL IN LISTS DLL_LIST)
        get_filename_component(DLL_NAME \"\${DLL}\" NAME)
        set(TARGET_DLL \"\${TARGET_DIR}/\${DLL_NAME}\")
        
        # 在复制前检查文件是否需要更新（比较MD5）
        set(NEEDS_COPY FALSE)
        if(EXISTS \"\${TARGET_DLL}\")
            # 目标文件存在，比较内容（MD5哈希）
            file(MD5 \"\${DLL}\" SRC_MD5)
            file(MD5 \"\${TARGET_DLL}\" DST_MD5)
            if(NOT \"\${SRC_MD5}\" STREQUAL \"\${DST_MD5}\")
                set(NEEDS_COPY TRUE)
            endif()
        else()
            # 目标文件不存在，需要复制
            set(NEEDS_COPY TRUE)
        endif()
        
        if(NEEDS_COPY)
            # 复制DLL文件
            execute_process(
                COMMAND \${CMAKE_COMMAND} -E copy_if_different \"\${DLL}\" \"\${TARGET_DIR}\"
                RESULT_VARIABLE COPY_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )
            
            if(COPY_RESULT EQUAL 0)
                math(EXPR COPIED_COUNT \"\${COPIED_COUNT} + 1\")
                list(APPEND COPIED_FILES \"  - \${DLL_NAME}\")
            else()
                message(WARNING \"复制DLL失败: \${DLL}\")
            endif()
            
            # 复制对应的.pdb文件（如果存在）
            string(REGEX REPLACE \"\\\\.dll$\" \".pdb\" PDB_FILE \"\${DLL}\")
            if(EXISTS \"\${PDB_FILE}\")
                execute_process(
                    COMMAND \${CMAKE_COMMAND} -E copy_if_different \"\${PDB_FILE}\" \"\${TARGET_DIR}\"
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
            endif()
        endif()
    endforeach()
    
    # 只在有实际复制操作时输出详细信息
    if(COPIED_COUNT GREATER 0)
        message(STATUS \"更新了 \${COPIED_COUNT} 个运行时DLL:\")
        foreach(FILE IN LISTS COPIED_FILES)
            message(STATUS \"\${FILE}\")
        endforeach()
    endif()
    # 如果没有复制，完全静默，不输出任何信息
else()
    # 只在第一次或变化时提示
    if(NOT EXISTS \"\${TARGET_DIR}/.dll_check_done\")
        message(STATUS \"未检测到运行时DLL依赖（全静态链接）\")
        file(WRITE \"\${TARGET_DIR}/.dll_check_done\" \"checked\")
    endif()
endif()
")
        
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_dlls_$<CONFIG>.cmake
            COMMENT "自动检测和复制运行时DLL"
        )
    else()
        message(WARNING "CMake版本 < 3.21，无法自动复制运行时DLL。")
        message(WARNING "如果使用了动态库，请手动复制DLL到exe目录。")
    endif()
endif()

# ============================================
# 项目配置完成提示
# ============================================
message(STATUS "")
message(STATUS "==============================================")
message(STATUS "项目配置完成！")
message(STATUS "  目标名称: ${TARGET}")
message(STATUS "  C++标准: C++${CMAKE_CXX_STANDARD}")
# 显示可执行文件输出目录
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    message(STATUS "  可执行文件输出: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
    # 在多配置生成器（如Visual Studio、Xcode）中，exe会在配置子目录中
    if(CMAKE_CONFIGURATION_TYPES)
        message(STATUS "  可执行文件输出: ${CMAKE_BINARY_DIR}/<配置名称> (如Debug/Release)")
    else()
        message(STATUS "  可执行文件输出: ${CMAKE_BINARY_DIR}")
    endif()
endif()
message(STATUS "  智能依赖获取: 预编译(prebuilt目录) > 系统库 > 本地源码(external目录) > 在线获取")
if(BUILD_SHARED_LIBS)
    message(STATUS "  依赖库默认链接: 动态链接 (Shared)")
else()
    message(STATUS "  依赖库默认链接: 静态链接 (Static)")
endif()
if(WIN32)
    message(STATUS "  运行时库处理: 自动检测并复制所有DLL到exe目录")
    if(MSVC)
        message(STATUS "  字符编码: UTF-8 (/utf-8) - 避免中文乱码")
    endif()
elseif(APPLE)
    message(STATUS "  运行时库处理: 使用RPATH (@executable_path)")
elseif(UNIX)
    message(STATUS "  运行时库处理: 使用RPATH ($ORIGIN)")
endif()
message(STATUS "==============================================")
message(STATUS "")