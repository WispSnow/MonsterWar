#include "context.h"
#include "time.h"
#include "game_state.h"
#include "engine/input/input_manager.h"
#include "engine/render/renderer.h"
#include "engine/render/camera.h"
#include "engine/render/text_renderer.h"
#include "engine/resource/resource_manager.h"
#include "engine/audio/audio_player.h"
#include <spdlog/spdlog.h>
#include <entt/signal/dispatcher.hpp>

namespace engine::core {

Context::Context(entt::dispatcher& dispatcher,
                 engine::input::InputManager& input_manager, 
                 engine::render::Renderer& renderer,
                 engine::render::Camera& camera,
                 engine::render::TextRenderer& text_renderer,
                 engine::resource::ResourceManager& resource_manager,
                 engine::audio::AudioPlayer& audio_player,
                 engine::core::GameState& game_state,
                 engine::core::Time& time)     
    : dispatcher_(dispatcher),
      input_manager_(input_manager),
      renderer_(renderer),
      camera_(camera),
      text_renderer_(text_renderer),
      resource_manager_(resource_manager),
      audio_player_(audio_player),
      game_state_(game_state),
      time_(time)
{
    spdlog::trace("上下文已创建并初始化。");
}

} // namespace engine::core 