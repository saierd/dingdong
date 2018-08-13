#pragma once

#include <spdlog/spdlog.h>

inline std::shared_ptr<spdlog::logger> log() {
    return spdlog::get("general");
}

inline std::shared_ptr<spdlog::logger> copyLoggerWithName(std::shared_ptr<spdlog::logger> const& logger, std::string const& name) {
    auto newLogger = std::make_shared<spdlog::logger>(name, logger->sinks().begin(), logger->sinks().end());
    newLogger->set_level(logger->level());
    return newLogger;
}

inline std::shared_ptr<spdlog::logger> categoryLogger(std::string const& name) {
    return copyLoggerWithName(log(), name);
}
