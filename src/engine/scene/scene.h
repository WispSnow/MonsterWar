#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <entt/entity/registry.hpp>

namespace engine::core {
    class Context;
}

namespace engine::ui {
    class UIManager;
}

namespace engine::scene {
    class SceneManager;

/**
 * @brief 场景基类，负责管理场景中的游戏对象和场景生命周期。
 *
 * 包含一组游戏对象，并提供更新、渲染、处理输入和清理的接口。
 * 派生类应实现具体的场景逻辑。
 */
class Scene {
protected:
    std::string scene_name_;                            ///< @brief 场景名称
    engine::core::Context& context_;                    ///< @brief 上下文引用（隐式，构造时传入）
    std::unique_ptr<engine::ui::UIManager> ui_manager_; ///< @brief UI管理器(初始化时自动创建)
    entt::registry registry_;                           ///< @brief ECS注册表
    
    bool is_initialized_ = false;                       ///< @brief 场景是否已初始化(非当前场景很可能未被删除，因此需要初始化标志避免重复初始化)

public:
    /**
     * @brief 构造函数。
     *
     * @param name 场景的名称。
     * @param context 场景上下文。
     */
    Scene(std::string_view name, engine::core::Context& context);

    virtual ~Scene();           // 1. 基类必须声明虚析构函数才能让派生类析构函数被正确调用。
                                // 2. 析构函数定义必须写在cpp中，不然需要引入GameObject头文件

    // 禁止拷贝和移动构造
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    // 核心循环方法
    virtual void init();                        ///< @brief 初始化场景。
    virtual void update(float delta_time);      ///< @brief 更新场景。
    virtual void render();                      ///< @brief 渲染场景。
    virtual void clean();                       ///< @brief 清理场景。

    /// @brief 请求弹出当前场景。
    void requestPopScene();

    /// @brief 请求压入一个新场景。
    void requestPushScene(std::unique_ptr<engine::scene::Scene>&& scene);
    
    /// @brief 请求替换当前场景。
    void requestReplaceScene(std::unique_ptr<engine::scene::Scene>&& scene);

    /// @brief 退出游戏。
    void quit();

    // getters and setters
    void setName(std::string_view name) { scene_name_ = name; }               ///< @brief 设置场景名称
    std::string_view getName() const { return scene_name_; }                  ///< @brief 获取场景名称
    void setInitialized(bool initialized) { is_initialized_ = initialized; }    ///< @brief 设置场景是否已初始化
    bool isInitialized() const { return is_initialized_; }                      ///< @brief 获取场景是否已初始化
    entt::registry& getRegistry() { return registry_; }                      ///< @brief 获取注册表引用

    engine::core::Context& getContext() const { return context_; }                  ///< @brief 获取上下文引用

};

} // namespace engine::scene