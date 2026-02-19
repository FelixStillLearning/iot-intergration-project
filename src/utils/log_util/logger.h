#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cstdlib>
#include <string>

/**
 * logger.h -- Utility untuk inisialisasi dan konfigurasi logging
 * 
 * File ini menyediakan fungsi untuk setup logger spdlog.
 * spdlog adalah library logging C++ yang cepat dan mudah digunakan.
 * 
 * Logger diinisialisasi paling awal di main() agar semua log
 * dari komponen lain (gRPC, DDS, WebSocket) bisa tercatat.
 * 
 * Log level bisa diatur via environment variable LOG_LEVEL:
 *   - trace    : log SEMUA (paling verbose)
 *   - debug    : detail teknis untuk development
 *   - info     : informasi umum (default)
 *   - warn     : peringatan, ada yang perlu perhatian
 *   - error    : kesalahan, ada yang gagal
 *   - critical : fatal, aplikasi mungkin crash
 * 
 * Format log: [tanggal waktu] [LEVEL] pesan
 * Contoh:     [2026-02-19 10:30:45.123] [info] DDS: Initialized successfully
 */
namespace utils {
    /**
     * Parse string log level menjadi enum spdlog level.
     * 
     * Digunakan untuk mengkonversi environment variable LOG_LEVEL
     * (yang berupa string) ke tipe enum yang dipahami spdlog.
     * 
     * @param level String nama level ("trace", "debug", "info", "warn", "error", "critical")
     * @return spdlog::level::level_enum yang sesuai, default ke info jika tidak dikenali
     */
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
        return spdlog::level::info;  // Default: info level
    }

    /**
     * Inisialisasi logger spdlog.
     * 
     * Konfigurasi:
     *   1. Set format output: [tanggal waktu.milidetik] [LEVEL] pesan
     *   2. Baca log level dari environment variable LOG_LEVEL
     *   3. Terapkan log level (default: info)
     * 
     * Harus dipanggil SEKALI di awal main() sebelum komponen lain diinisialisasi.
     */
    inline void init_logger() {
        // Set format: [tahun-bulan-tanggal jam:menit:detik.milidetik] [LEVEL] pesan
        // %^ dan %$ menandai bagian yang diberi warna (hanya level)
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        // Baca log level dari environment variable, default "info"
        const char* level_env = std::getenv("LOG_LEVEL");
        std::string level = level_env ? std::string(level_env) : "info";
        spdlog::set_level(parse_log_level(level));  // Terapkan level

        spdlog::info("Logger module initialized (level: {})", level);
    }
}
