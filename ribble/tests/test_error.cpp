#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <ribble/error/error.h>
#include <ribble/error/verbose_error.h>

enum class TestErrorCode : uint8_t {
    SUCCESS = 0,
    INVALID_INPUT = 1,
    OUT_OF_MEMORY = 2,
    FILE_NOT_FOUND = 3
};

TEST_SUITE("Error System") {
    TEST_CASE("Basic Error Creation") {
        ribble::error::Error error(static_cast<uint8_t>(TestErrorCode::INVALID_INPUT), false);
        
        CHECK(error.failure<TestErrorCode>() == TestErrorCode::INVALID_INPUT);
        CHECK_FALSE(error.is_fatal());
    }

    TEST_CASE("Fatal Error") {
        ribble::error::Error fatalError(static_cast<uint8_t>(TestErrorCode::OUT_OF_MEMORY), true);
        
        CHECK(fatalError.failure<TestErrorCode>() == TestErrorCode::OUT_OF_MEMORY);
        CHECK(fatalError.is_fatal());
    }

    TEST_CASE("Error with Location") {
        #ifdef RIBBLE_DEBUG
        ribble::error::Error::ErrorLocation loc{__FILE__, __LINE__};
        ribble::error::Error error(static_cast<uint8_t>(TestErrorCode::FILE_NOT_FOUND), loc, false);
        #else
        ribble::error::Error::ErrorLocation loc{};
        ribble::error::Error error(static_cast<uint8_t>(TestErrorCode::FILE_NOT_FOUND), loc, false);
        #endif
        
        CHECK(error.failure<TestErrorCode>() == TestErrorCode::FILE_NOT_FOUND);
        
        auto location = error.location();
        #ifdef RIBBLE_DEBUG
        REQUIRE(location.has_value());
        CHECK(location->file != nullptr);
        CHECK(location->line > 0);
        #else
        CHECK_FALSE(location.has_value());
        #endif
    }

    TEST_CASE("VerboseError Creation") {
        #ifdef RIBBLE_DEBUG
        // In debug mode, message is stored
        ribble::error::VerboseError error(
            static_cast<uint8_t>(TestErrorCode::INVALID_INPUT),
            std::string("Test error message")
        );
        CHECK(error.message() == "Test error message");
        #else
        // In release mode, const char* can be passed directly (no allocation)
        ribble::error::VerboseError error(
            static_cast<uint8_t>(TestErrorCode::INVALID_INPUT),
            "Test error message"
        );
        CHECK(error.message().empty());
        #endif
        
        CHECK(error.failure<TestErrorCode>() == TestErrorCode::INVALID_INPUT);
        CHECK_FALSE(error.is_fatal());
    }

    TEST_CASE("VerboseError with Location") {
        #ifdef RIBBLE_DEBUG
        ribble::error::VerboseError error(
            static_cast<uint8_t>(TestErrorCode::OUT_OF_MEMORY),
            __FILE__,
            __LINE__,
            std::string("Memory allocation failed")
        );
        CHECK(error.message() == "Memory allocation failed");
        #else
        ribble::error::VerboseError error(
            static_cast<uint8_t>(TestErrorCode::OUT_OF_MEMORY),
            __FILE__,
            __LINE__,
            "Memory allocation failed"
        );
        CHECK(error.message().empty());
        #endif
        
        CHECK(error.failure<TestErrorCode>() == TestErrorCode::OUT_OF_MEMORY);
        
        auto location = error.location();
        #ifdef RIBBLE_DEBUG
        REQUIRE(location.has_value());
        CHECK(location->file != nullptr);
        CHECK(location->line > 0);
        #else
        CHECK_FALSE(location.has_value());
        #endif
    }

    TEST_CASE("Error Code Conversion") {
        ribble::error::Error error(42, false);
        
        // Should work with enum types
        CHECK(error.failure<TestErrorCode>() == static_cast<TestErrorCode>(42));
        
        // Should work with uint8_t
        CHECK(error.failure<uint8_t>() == 42);
    }

    TEST_CASE("VerboseError Efficiency - No String Allocation in Release") {
        // This test verifies that VerboseError can be created with string literals
        // without requiring std::string allocation in release mode
        const char* msg = "Direct string literal";
        ribble::error::VerboseError error(
            static_cast<uint8_t>(TestErrorCode::FILE_NOT_FOUND),
            msg
        );
        
        CHECK(error.failure<TestErrorCode>() == TestErrorCode::FILE_NOT_FOUND);
        
        // In release mode, message is empty but no allocation occurred
        // In debug mode, message would be stored (but we're testing release efficiency)
        #ifndef RIBBLE_DEBUG
        CHECK(error.message().empty());
        #endif
    }

    TEST_SUITE("Error Callback System") {
        TEST_CASE("SetCallback and Error::Throw") {
            bool callbackCalled = false;
            uint8_t receivedFailure = 0;
            bool receivedFatal = false;

            ribble::error::Error::SetCallback([&](const ribble::error::Error& error) {
                callbackCalled = true;
                receivedFailure = error.failure<uint8_t>();
                receivedFatal = error.is_fatal();
            });

            ribble::error::Error::Throw(
                static_cast<uint8_t>(TestErrorCode::INVALID_INPUT),
                false
            );

            CHECK(callbackCalled);
            CHECK(receivedFailure == static_cast<uint8_t>(TestErrorCode::INVALID_INPUT));
            CHECK_FALSE(receivedFatal);

            // Reset callback
            ribble::error::Error::SetCallback(nullptr);
        }

        TEST_CASE("Error::Throw with Location") {
            bool callbackCalled = false;
            #ifdef RIBBLE_DEBUG
            const char* receivedFile = nullptr;
            size_t receivedLine = 0;
            #endif

            ribble::error::Error::SetCallback([&](const ribble::error::Error& error) {
                callbackCalled = true;
                #ifdef RIBBLE_DEBUG
                auto loc = error.location();
                if (loc.has_value()) {
                    receivedFile = loc->file;
                    receivedLine = loc->line;
                }
                #else
                (void)error; // Suppress unused parameter warning in release mode
                #endif
            });

            #ifdef RIBBLE_DEBUG
            ribble::error::Error::ErrorLocation loc{__FILE__, __LINE__};
            ribble::error::Error::Throw(
                static_cast<uint8_t>(TestErrorCode::FILE_NOT_FOUND),
                loc,
                true
            );
            #else
            ribble::error::Error::ErrorLocation loc{};
            ribble::error::Error::Throw(
                static_cast<uint8_t>(TestErrorCode::FILE_NOT_FOUND),
                loc,
                true
            );
            #endif

            CHECK(callbackCalled);
            #ifdef RIBBLE_DEBUG
            CHECK(receivedFile != nullptr);
            CHECK(receivedLine > 0);
            #endif

            ribble::error::Error::SetCallback(nullptr);
        }

        TEST_CASE("VerboseError::Throw") {
            bool callbackCalled = false;
            bool isVerboseError = false;

            ribble::error::Error::SetCallback([&](const ribble::error::Error& error) {
                callbackCalled = true;
                // Try to cast to VerboseError to verify it's actually a VerboseError
                const auto* verbose = dynamic_cast<const ribble::error::VerboseError*>(&error);
                isVerboseError = (verbose != nullptr);
            });

            #ifdef RIBBLE_DEBUG
            ribble::error::VerboseError::Throw(
                static_cast<uint8_t>(TestErrorCode::OUT_OF_MEMORY),
                std::string("Test message")
            );
            #else
            ribble::error::VerboseError::Throw(
                static_cast<uint8_t>(TestErrorCode::OUT_OF_MEMORY),
                "Test message"
            );
            #endif

            CHECK(callbackCalled);
            CHECK(isVerboseError);

            ribble::error::Error::SetCallback(nullptr);
        }

        TEST_CASE("VerboseError::Throw with Location") {
            bool callbackCalled = false;

            ribble::error::Error::SetCallback([&](const ribble::error::Error& error) {
                callbackCalled = true;
                CHECK(error.failure<TestErrorCode>() == TestErrorCode::INVALID_INPUT);
            });

            #ifdef RIBBLE_DEBUG
            ribble::error::VerboseError::Throw(
                static_cast<uint8_t>(TestErrorCode::INVALID_INPUT),
                __FILE__,
                __LINE__,
                std::string("Location test")
            );
            #else
            ribble::error::VerboseError::Throw(
                static_cast<uint8_t>(TestErrorCode::INVALID_INPUT),
                __FILE__,
                __LINE__,
                "Location test"
            );
            #endif

            CHECK(callbackCalled);

            ribble::error::Error::SetCallback(nullptr);
        }

        TEST_CASE("Throw without Callback") {
            // Should not crash when callback is not set
            ribble::error::Error::SetCallback(nullptr);
            
            ribble::error::Error::Throw(
                static_cast<uint8_t>(TestErrorCode::FILE_NOT_FOUND),
                false
            );

            // If we get here without crashing, the test passes
            CHECK(true);
        }

        TEST_CASE("Callback Replacement") {
            int callCount = 0;

            // Set first callback
            ribble::error::Error::SetCallback([&]([[maybe_unused]] const ribble::error::Error&) {
                callCount = 1;
            });
            ribble::error::Error::Throw(1, false);
            CHECK(callCount == 1);

            // Replace with second callback
            ribble::error::Error::SetCallback([&]([[maybe_unused]] const ribble::error::Error&) {
                callCount = 2;
            });
            ribble::error::Error::Throw(1, false);
            CHECK(callCount == 2);

            ribble::error::Error::SetCallback(nullptr);
        }
    }
}

