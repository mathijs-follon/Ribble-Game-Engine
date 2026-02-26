#include <ribble.h>

using namespace ribble::core;
using namespace ribble;
using namespace std::chrono_literals;


int main() {
    Engine engine;

    if (const auto result = engine.initialize(); !result) {
        RIBBLE_LOG_ERROR("Failed to initialize correctly.");
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
