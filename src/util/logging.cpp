#include "logging.h"

#include <spdlog/sinks/stdout_color_sinks.h>

class GlobalLogger {
public:
    GlobalLogger() {
        logger = spdlog::stdout_color_mt("general");
    }

    void setLogLevel(spdlog::level::level_enum level) const {
        logger->set_level(level);
    }

    void setLogLevel(std::string const& levelString) const {
        auto level = spdlog::level::from_str(levelString);
        setLogLevel(level);
    }

private:
    Logger logger;
};

static GlobalLogger globalLogger;

void setLogLevel(std::string const& level) {
    globalLogger.setLogLevel(level);
}
