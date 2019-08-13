#include "rfid_scanner.h"

#include <cctype>
#include <thread>

#include "system/external_process.h"
#include "util/logging.h"

std::string const rfidLoggingCategory = "rfid";

class RfidScanner::Impl {
public:
    std::thread scanThread;
};

RfidScanner::RfidScanner(std::string const& script) {
    impl = std::make_unique<Impl>();
    impl->scanThread = std::thread([this, script]() {
        auto logger = categoryLogger(rfidLoggingCategory);
        ExternalProcess scanProcess(script);

        while (true) {
            std::string key = scanProcess.readLine();
            if (!key.empty()) {
                // Sometimes, the scanner outputs some error codes like "E2" and similar things, skip those.
                bool isNumeric = true;
                for (auto const& character : key) {
                    if (!std::isdigit(character)) {
                        isNumeric = false;
                        break;
                    }
                }

                if (isNumeric) {
                    logger->trace("Scanned key {}", key);
                    onKeyScanned.emit(std::move(key));
                }
            }
            if (!scanProcess.isRunning()) {
                logger->error("RFID scanning process stopped working");
                break;
            }
        }
    });
}

RfidScanner::~RfidScanner() = default;
