#include "camera.h"

#include "util/logging.h"

#include <procxx/process.h>

std::string const takeImageCommand = "./scripts/take_image.sh";

std::string cameraLoggingCategory = "camera";

std::vector<std::uint8_t> takeCameraImage() {
#ifdef RASPBERRY_PI
    try {
        procxx::process process(takeImageCommand);
        process.exec();

        std::vector<std::uint8_t> result(std::istreambuf_iterator<char>(process.output()),
                                         std::istreambuf_iterator<char>());
        if (result.size() < 2 || result[0] != 0xff || result[1] != 0xd8) {
            return {};
        }
        return result;
    } catch (...) {
        categoryLogger(cameraLoggingCategory)->error("Error while capturing camera image");
    }
#endif

    return {};
}
