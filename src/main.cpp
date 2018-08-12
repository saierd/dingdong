#include "stream/audio.h"
#include "stream/gstreamer.h"
#include "stream/video.h"

#include <iostream>
#include "discovery.h"
#include "settings.h"

#include <glibmm/dispatcher.h>

#include "ui/main_window.h"

int main(int argc, char** argv) {
    auto app = Gtk::Application::create(argc, argv, "org.dingdong");
    initializeGStreamer(argc, argv);

    Settings self("Test Instance");
    InstanceDiscovery discovery(self);

    MainWindow mainWindow;

    Glib::Dispatcher instancesChangedSignal;

    instancesChangedSignal.connect([&discovery, &mainWindow]() {
        for (auto const& instance : discovery.instances()) {
            std::cout << "Instance " << instance.name() << std::endl;
            std::cout << "\tID " << instance.id().toString() << std::endl;
            std::cout << "\tIP " << instance.ipAddress().toString() << std::endl;
        }
        mainWindow.updateInstances(discovery.instances());
    });
    discovery.onInstancesChanged([&instancesChangedSignal](std::vector<Instance> const&) {
        instancesChangedSignal();
    });

    mainWindow.onCall.connect([](Instance const& instance) {
        std::cout << "Call " << instance.name() << std::endl;
    });

    return app->run(mainWindow);



//    std::string const targetHost = "localhost";
    std::string const targetHost = "192.168.178.35";
    int audioPort = 5555;
    int videoPort = 5556;

#if 1
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