#pragma once

#include <memory>
#include <string>

#include <glibmm.h>

class RfidScanner {
public:
    RfidScanner();
    ~RfidScanner();

    sigc::signal<void, std::string> onKeyScanned;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
