#pragma once

#include <string>

struct _GstElement;
using GstElement = struct _GstElement;

class Pipeline {
public:
    explicit Pipeline(std::string const& specification);
    ~Pipeline();

    void start();
    void stop();

    bool isRunning() const;

    GstElement* getElementByName(std::string const& name);

private:
    GstElement* pipeline = nullptr;
    bool running = false;
};
