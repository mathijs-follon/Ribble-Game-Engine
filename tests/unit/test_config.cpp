#include <doctest/doctest.h>

#include "ribble/core/config.h"

TEST_CASE("Empty config returns error on get") {
    ribble::core::DefaultConfig config{};

    auto result = config.get<int>("non existing");

    CHECK_FALSE(result.has_value());
    CHECK_EQ(result.error().failureType, ribble::core::ConfigFailure::InvalidKeyAccess);
}

TEST_CASE("Set and get values") {
    ribble::core::DefaultConfig config{};

    config.set("int_value", 42);
    config.set("float_value", 3.14f);
    config.set("bool_value", true);
    config.set("string_value", std::string("hello"));

    CHECK_EQ(config.get<int>("int_value").value(), 42);
    CHECK_EQ(config.get<float>("float_value").value(), doctest::Approx(3.14f));
    CHECK_EQ(config.get<bool>("bool_value").value(), true);
    CHECK_EQ(config.get<std::string>("string_value").value(), "hello");
}

TEST_CASE("Get with wrong type returns error") {
    ribble::core::DefaultConfig config{};

    config.set("value", 123);

    auto result = config.get<float>("value");

    CHECK_FALSE(result.has_value());
    CHECK_EQ(result.error().code(), ribble::core::ConfigFailure::BadVariantAccess);
}

TEST_CASE("Set overwrites existing key") {
    ribble::core::DefaultConfig config{};

    config.set("value", 10);
    CHECK_EQ(config.get<int>("value").value(), 10);

    config.set("value", 20);
    CHECK_EQ(config.get<int>("value").value(), 20);
}

TEST_CASE("Load config from JSON") {
    using nlohmann::json;

    ribble::core::DefaultConfig config{};

    json j = {
        {"width", 800},
        {"height", 600},
        {"title", "App"},
        {"fullscreen", false},
        {"scale", 1.5f}
    };

    auto result = config.load_from_json(j);

    CHECK(result.has_value());
    CHECK_EQ(config.get<int>("width").value(), 800);
    CHECK_EQ(config.get<int>("height").value(), 600);
    CHECK_EQ(config.get<std::string>("title").value(), "App");
    CHECK_EQ(config.get<bool>("fullscreen").value(), false);
    CHECK_EQ(config.get<float>("scale").value(), doctest::Approx(1.5f));
}

TEST_CASE("Load JSON fails when root is not object") {
    using nlohmann::json;

    ribble::core::DefaultConfig config{};

    json j = json::array({1, 2, 3});

    auto result = config.load_from_json(j);

    CHECK_FALSE(result.has_value());
    CHECK_EQ(result.error().code(), ribble::core::ConfigFailure::JSONFileParseFailure);
}

TEST_CASE("Load JSON fails on unsupported value type") {
    using nlohmann::json;

    ribble::core::DefaultConfig config{};

    json j = {
        {"valid", 123},
        {"invalid", json::array({1, 2, 3})}
    };

    auto result = config.load_from_json(j);

    CHECK_FALSE(result.has_value());
    CHECK_EQ(result.error().code(), ribble::core::ConfigFailure::BadVariantAccess);
}

TEST_CASE("Load JSON stops on first invalid entry") {
    using nlohmann::json;

    ribble::core::DefaultConfig config{};

    json j = {
        {"ok", 42},
        {"bad", json::object()}
    };

    auto result = config.load_from_json(j);

    CHECK_FALSE(result.has_value());
    CHECK_FALSE(config.get<int>("ok").has_value());
}