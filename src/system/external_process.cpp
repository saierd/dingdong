#include "external_process.h"

#include <array>

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
    return std::feof(stream) != 0;
}

std::string ExternalProcess::readLine() const {
    std::array<char, bufferSize> buffer;

    std::string line;
    while (std::fgets(buffer.data(), bufferSize, stream) != nullptr) {
        line += std::string(buffer.data());
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
            break;
        }
    }

    return line;
}

void runExternalProcess(std::string command) {
    ExternalProcess process(std::move(command));
}
