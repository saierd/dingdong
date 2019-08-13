#include "external_process.h"

int const bufferSize = 1024;

ExternalProcess::ExternalProcess(std::string command) {
    // Redirect stderr, because popen can only read stdout.
    command += " 2>&1";

    stream = popen(command.c_str(), "r");
}

ExternalProcess::~ExternalProcess() {
    if (stream) {
        pclose(stream);
    }
}

bool ExternalProcess::isRunning() const {
    if (!stream) return false;
    return !std::feof(stream);
}

std::string ExternalProcess::readLine() const {
    char buffer[bufferSize];

    std::string line;
    while (std::fgets(buffer, bufferSize, stream) != nullptr) {
        line += std::string(buffer);
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
            break;
        }
    }

    return line;
}
