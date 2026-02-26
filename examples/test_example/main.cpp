#include <ribble.h>

using namespace ribble::core;
using namespace ribble;
using namespace backend;

int main() {
    Engine engine;

    EngineContextSettings settings;
    settings.graphics.renderBackend = RenderBackendType::OpenGL;
    settings.window.windowWidth = 1400;
    settings.window.windowHeight = 700;
    settings.window.windowTitle = "This Is My Window";

    if (const auto result = engine.initialize(settings); !result) {
        RIBBLE_LOG_ERROR("Failed to initialize correctly.");
        std::exit(-1);
    }

    if (const auto result = engine.create_window(); !result) {
        RIBBLE_LOG_ERROR("Failed to create window.");
        std::exit(-1);
    }


    if (const auto result = engine.run(); !result) {
        RIBBLE_LOG_ERROR("Failed to run correctly.");
        std::exit(-1);
    }

    if (const auto result = engine.shutdown(); !result) {
        RIBBLE_LOG_ERROR("Failed to shut down correctly.");
        std::exit(-1);
    }
}
