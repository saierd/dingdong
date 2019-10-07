#pragma once

#include <cstdio>
#include <string>

class ExternalProcess {
public:
    ExternalProcess(std::string command);
    ~ExternalProcess();

    bool isRunning() const;
    std::string readLine() const;

private:
    std::FILE* stream;
};

void runExternalProcess(std::string command);
