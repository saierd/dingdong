#include "logging.h"

#include <spdlog/sinks/stdout_color_sinks.h>

class GlobalLogger {
public:
    GlobalLogger() {
        logger = spdlog::stdout_color_mt("general");
#ifndef NDEBUG
        logger->set_level(spdlog::level::trace);
#endif
    }

private:
    std::shared_ptr<spdlog::logger> logger;
};

static GlobalLogger globalLogger;
