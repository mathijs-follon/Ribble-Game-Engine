#include <expected>
#include <ribble.h>

#include "ribble/core/config.h"


using namespace ribble::core;
using namespace ribble;

enum class TestFailure {
    IShouldFail
};
RIBBLE_ENUM_TO_STRING(TestFailure,
    case TestFailure::IShouldFail: return "I Was Told To Fail";
);

Result<int, TestFailure> get_int(bool failMe) {
    if (failMe) {
        return Fail( RIBBLE_ERROR(TestFailure::IShouldFail, "The get_int function failed") );
    }
    return Ok(4);
}

int main() {
    InitializeLogger(
        "ExampleLogs"
    );

    auto result = get_int(true);
    if (result.has_value()) {
        RIBBLE_LOG_INFO("We got an int! {}", result.value());
    } else {
        RIBBLE_LOG_DEBUG("Oops we didnt got an int!");
    }
}
