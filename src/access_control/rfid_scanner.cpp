#include "rfid_scanner.h"

#include <cctype>
#include <thread>

#include <procxx/process.h>

#include "util/logging.h"

std::string const rfidLoggingCategory = "rfid";

std::string const rfidScannerCommand = "./scripts/read_rfid.py";

class RfidScanner::Impl {
public:
    std::thread scanThread;
};

RfidScanner::RfidScanner() {
    impl = std::make_unique<Impl>();
    impl->scanThread = std::thread([this]() {
        auto logger = categoryLogger(rfidLoggingCategory);
        procxx::process scanProcess(rfidScannerCommand);
        scanProcess.exec();

        while (true) {
            std::string key;
            std::getline(scanProcess.output(), key);
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
                    onKeyScanned.emit(key);
                }
            }
            if (!scanProcess.running()) {
                logger->error("RFID scanning process stopped working");

                // Try to restart the process.
                scanProcess.wait();
                scanProcess.exec();
            }
        }
    });
}

RfidScanner::~RfidScanner() = default;
