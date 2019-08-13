#pragma once

#include <spdlog/spdlog.h>

using Logger = std::shared_ptr<spdlog::logger>;

inline Logger log() {
    return spdlog::get("general");
}

inline Logger copyLoggerWithName(Logger const& logger, std::string const& name) {
    auto newLogger = std::make_shared<spdlog::logger>(name, logger->sinks().begin(), logger->sinks().end());
    newLogger->set_level(logger->level());
    return newLogger;
}

inline Logger categoryLogger(std::string const& name) {
    return copyLoggerWithName(log(), name);
}

void setLogLevel(std::string const& level);
