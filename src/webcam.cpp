#include "webcam.h"

#include "util/logging.h"

#include <procxx/process.h>

#include <thread>

std::string const restartWebcamCommand = "./scripts/restart_webcam.sh";

static std::thread restartThread;

void restartWebcam() {
    waitForWebcam();

#ifdef RASPBERRY_PI
    restartThread = std::thread([]() {
        categoryLogger("webcam")->debug("Restarting webcam");
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            procxx::process(restartWebcamCommand).exec();

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            procxx::process(restartWebcamCommand).exec();
        } catch (procxx::process::exception& e) {
            categoryLogger("webcam")->error(e.what());
        }
    });
#endif
}

void waitForWebcam() {
#ifdef RASPBERRY_PI
    if (restartThread.joinable()) {
        categoryLogger("webcam")->debug("Waiting for webcam");
        restartThread.join();
        categoryLogger("webcam")->debug("Webcam is available");
    }
#endif
}

bool fileExists(std::string const& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

std::string getWebcamDevice() {
    return "/dev/video0";

#ifdef RASPBERRY_PI
    // On Raspberry Pi, /dev/video1 is usually the onboard camera. /dev/video0 is the USB webcam. After restarting the
    // webcam (see above), it might get a different number, though.
    for (auto const& name : { "/dev/video0", "/dev/video2" }) {
        if (fileExists(name)) {
            categoryLogger("webcam")->debug("Using video device {}", name);
            return name;
        }
    }
#endif

    return "";
}
