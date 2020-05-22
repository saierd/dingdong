#include "webcam.h"

#include "util/logging.h"

#include <procxx/process.h>

#include <atomic>
#include <random>
#include <thread>

std::string const restartWebcamCommand = "./scripts/restart_webcam.sh";
std::string const videoDevice = "/dev/video0";

struct WebcamRestartData {
    std::atomic<bool> stopRestarting = false;
    std::thread thread;

public:
    std::chrono::milliseconds generateRandomWaitTime() {
        return std::chrono::milliseconds(waitTimeDistribution(rng));
    }

private:
    std::mt19937 rng;
    std::uniform_int_distribution<> waitTimeDistribution{ 300, 700 };
};

static WebcamRestartData webcamRestartData;

bool fileExists(std::string const& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void _restartWebcamOnce() {
    // Wait a bit until the webcam is closed in case it was just used.
    std::this_thread::sleep_for(webcamRestartData.generateRandomWaitTime());

    categoryLogger("webcam")->debug("Restarting webcam");
    try {
        procxx::process(restartWebcamCommand).exec();
    } catch (procxx::process::exception& e) {
        categoryLogger("webcam")->error(e.what());
    }
}

void _restartWebcam() {
    _restartWebcamOnce();

    // Sometimes the restarted webcam will fail to connect as the correct device. In that case we cannot do anything but
    // to try restarting again. We do this until it works properly or there is a call. In that case we will stop
    // restarting (video will not work for that call) and hope that it works next time...
    while (!fileExists(videoDevice) && !webcamRestartData.stopRestarting) {
        _restartWebcamOnce();
    }
}

void restartWebcam() {
    waitForWebcam();

    webcamRestartData.stopRestarting = false;
#ifdef RASPBERRY_PI
    webcamRestartData.thread = std::thread(_restartWebcam);
#endif
}

void waitForWebcam() {
    if (webcamRestartData.thread.joinable()) {
        categoryLogger("webcam")->debug("Waiting for webcam");
        webcamRestartData.stopRestarting = true;
        webcamRestartData.thread.join();
        categoryLogger("webcam")->debug("Webcam is available");
    }
}
