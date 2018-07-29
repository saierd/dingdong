#include "stream/audio.h"
#include "stream/gstreamer.h"
#include "stream/video.h"

int main(int argc, char** argv) {
    initializeGStreamer(argc, argv);

    std::string const targetHost = "localhost";
//    std::string const targetHost = "192.168.178.26";
    int audioPort = 5555;
    int videoPort = 5556;

#if 0
    AudioSender audioSender(targetHost, audioPort);
    audioSender.start();
    VideoSender videoSender(targetHost, videoPort);
    videoSender.start();
#else
    AudioReceiver audioReceiver(audioPort);
    audioReceiver.start();
    VideoReceiver videoReceiver(videoPort);
    videoReceiver.start();
#endif

    GStreamerMainLoop mainLoop;
    mainLoop.run();
}
