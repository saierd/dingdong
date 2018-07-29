#pragma once

#include <memory>

void initializeGStreamer(int argc, char** argv);

class GStreamerMainLoop {
public:
    GStreamerMainLoop();
    ~GStreamerMainLoop();

    void run();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
