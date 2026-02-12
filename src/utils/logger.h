#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace utils {
    inline void init_logger() {
        // Pattern: [Jam:Menit:Detik.mili] [Level] Pesan
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        spdlog::set_level(spdlog::level::info);
        spdlog::info("Logger module initialized");
    }
}