#pragma once

#include <string>

struct _GstElement;
typedef struct _GstElement GstElement;

class Pipeline {
public:
    explicit Pipeline(std::string const& specification);
    ~Pipeline();

    void start();
    void stop();

    bool isRunning() const;

private:
    GstElement* pipeline = nullptr;
    bool running = false;
};
