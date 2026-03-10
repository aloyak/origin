#include "engine/debug/logger.h"

#include <spdlog/spdlog.h>
#include <iostream>

void Logger::setVerbose(int verbose) {
    switch (verbose) {
        case 0: spdlog::set_level(spdlog::level::off);   break;
        case 1: spdlog::set_level(spdlog::level::err);   break;
        case 2: spdlog::set_level(spdlog::level::warn);  break;
        case 3: spdlog::set_level(spdlog::level::info);  break;
        case 4: spdlog::set_level(spdlog::level::debug); break;
        case 5: spdlog::set_level(spdlog::level::trace); break;
        default: spdlog::set_level(spdlog::level::info); break;
    }
}

void Logger::clear() {
    std::cout << "\033[2J\033[H" << std::flush;
}


void Logger::log(const std::string& message) {
    Logger::info(message);
}

void Logger::debug(const std::string& message) {
    spdlog::debug(message);
}

void Logger::info(const std::string& message) {
    spdlog::info(message);
}

void Logger::warn(const std::string& message) {
    spdlog::warn(message);
}

void Logger::error(const std::string& message) {
    spdlog::error(message);
}