#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <glibmm.h>

class CallHistory {
public:
    struct Entry {
        bool accepted = false;
        bool canceled = false;
        std::string image;  // Image encoded as JPEG.

        bool missed() const {
            return !accepted && !canceled;
        }
    };

public:
    explicit CallHistory(int maxEntries = 10);

    void addEntry(bool accepted, bool canceled, std::string image);
    void clear();

    int numMissedEntries() const;
    std::vector<std::shared_ptr<Entry>> getMissedEntries() const;

    sigc::signal<void> onEntriesChanged;

private:
    int maxEntries;

    mutable std::mutex mutex;
    std::deque<std::shared_ptr<Entry>> entries;
};
