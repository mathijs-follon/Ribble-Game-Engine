#include <ribble.h>

using namespace ribble::core;
using namespace ribble;
using namespace backend;
using namespace std::chrono_literals;

constexpr WindowBackendType wbt = WindowBackendType::X11;
constexpr RenderBackendType rbt = RenderBackendType::OpenGL;

int main() {
    Engine engine;

    if (const auto result = engine.initialize(wbt, rbt); !result) {
        RIBBLE_LOG_ERROR("Failed to initialize correctly.");
        std::exit(-1);
    }


    if (const auto result = engine.create_window(1400, 700, "This Is My Window"); !result) {
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
