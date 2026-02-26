#pragma once

#include <SDL3/SDL.h>
#include "../../common/window_backend.h"
#include <utility>

class SDLWindow : public ribble::backend::WindowBackend {
public:
    SDLWindow(std::shared_ptr<ribble::core::EventBus> windowEventBus)
    : WindowBackend{std::move(windowEventBus)}
    {}

    ribble::core::Result<void, Failure> initialize(int width, int height, const char *title) override;

    ribble::core::Result<void, Failure> poll_events() override;

    ribble::core::Result<void, Failure> shutdown() override;

    [[nodiscard]] void * native_handle() const override;

private:
    SDL_Window* m_window = nullptr;
    bool m_shouldClose = false;
};


