#pragma once
#include <forward_list>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unordered_map>
#include <variant>

#include "fail.h"


namespace ribble::core {
    template<typename T>
    struct JsonValueParser;

    template<>
    struct JsonValueParser<int> {
        static bool can_parse(const nlohmann::json &j) { return j.is_number_integer(); }
        static int parse(const nlohmann::json &j) { return j.get<int>(); }
    };

    template<>
    struct JsonValueParser<float> {
        static bool can_parse(const nlohmann::json &j) { return j.is_number(); }
        static float parse(const nlohmann::json &j) { return j.get<float>(); }
    };

    template<>
    struct JsonValueParser<bool> {
        static bool can_parse(const nlohmann::json &j) { return j.is_boolean(); }
        static bool parse(const nlohmann::json &j) { return j.get<bool>(); }
    };

    template<>
    struct JsonValueParser<std::string> {
        static bool can_parse(const nlohmann::json &j) { return j.is_string(); }
        static std::string parse(const nlohmann::json &j) { return j.get<std::string>(); }
    };

    enum class ConfigFailure {
        BadVariantAccess,
        InvalidKeyAccess,
        JSONFileParseFailure,
        Unknown,
    };

    template<typename K, typename... Args>
    class Config {
        std::unordered_map<K, std::variant<Args...>> m_map;

    public:
        template<typename T>
        Result<T, ConfigFailure> get(const K &key) const {
            auto it = m_map.find(key);
            if (it == m_map.end()) {
                return Fail(RIBBLE_ERROR(ConfigFailure::InvalidKeyAccess,
                                         "The requested key has no assigned value in the config."));
            }

            if (auto value = std::get_if<T>(&it->second)) {
                return Ok(*value);
            }

            return Fail(
                    RIBBLE_ERROR(ConfigFailure::BadVariantAccess, "The requested key has a different type than T."));
        }

        template<typename T>
        void set(const K &key, T &&value) {
            static_assert((std::is_same_v<T, Args> || ...), "Type not allowed in Config variant");
            m_map[key] = std::variant<Args...>(std::forward<T>(value));
        }


        Result<void, ConfigFailure> load_from_json(const nlohmann::json &j) {
            if (!j.is_object()) {
                return Fail(RIBBLE_ERROR(ConfigFailure::JSONFileParseFailure, "JSON root must be an object."));
            }

            for (auto &[key, value]: j.items()) {
                bool parsed = false;

                (
                        [&] {
                            if (!parsed && JsonValueParser<Args>::can_parse(value)) {
                                set(key, JsonValueParser<Args>::parse(value));
                                parsed = true;
                            }
                        }(),
                        ...);

                if (!parsed) {
                    return Fail(RIBBLE_ERROR(ConfigFailure::BadVariantAccess,
                                             "JSON value type does not match any Config variant type."));
                }
            }

            return Ok();
        }
    };

    class DefaultConfig : public Config<std::string, bool, int, float, std::string> {};
} // namespace ribble::core

RIBBLE_ENUM_TO_STRING(ribble::core::ConfigFailure,
                      case ribble::core::ConfigFailure::BadVariantAccess : return "Bad Variant";
                      case ribble::core::ConfigFailure::InvalidKeyAccess : return "Invalid Config Key";
                      case ribble::core::ConfigFailure::JSONFileParseFailure : return "Json Parse Failure";
                      case ribble::core::ConfigFailure::Unknown : return "Unknown";);
