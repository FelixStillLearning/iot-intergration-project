#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cstdlib>
#include <string>

namespace utils {
    inline spdlog::level::level_enum parse_log_level(const std::string& level) {
        if (level == "trace") {
            return spdlog::level::trace;
        }
        if (level == "debug") {
            return spdlog::level::debug;
        }
        if (level == "warn") {
            return spdlog::level::warn;
        }
        if (level == "error") {
            return spdlog::level::err;
        }
        if (level == "critical") {
            return spdlog::level::critical;
        }
        return spdlog::level::info;
    }

    inline void init_logger() {
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        const char* level_env = std::getenv("LOG_LEVEL");
        std::string level = level_env ? std::string(level_env) : "info";
        spdlog::set_level(parse_log_level(level));

        spdlog::info("Logger module initialized (level: {})", level);
    }
}
