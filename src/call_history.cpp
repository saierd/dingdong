#include "call_history.h"

CallHistory::CallHistory(int _maxEntries) : maxEntries(_maxEntries) {}

void CallHistory::addEntry(bool accepted, bool canceled, std::string image) {
    if (image.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);

    entries.push_front(std::make_shared<Entry>(Entry{ accepted, canceled, std::move(image) }));
    while (static_cast<int>(entries.size()) > maxEntries) {
        entries.pop_back();
    }

    onEntriesChanged.emit();
}

void CallHistory::clear() {
    std::lock_guard<std::mutex> lock(mutex);

    entries.clear();
}

int CallHistory::numMissedEntries() const {
    std::lock_guard<std::mutex> lock(mutex);

    int numMissed = 0;
    for (auto const& entry : entries) {
        if (entry->missed()) {
            numMissed++;
        }
    }

    return numMissed;
}

std::vector<std::shared_ptr<CallHistory::Entry>> CallHistory::getMissedEntries() const {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<std::shared_ptr<CallHistory::Entry>> result;
    result.reserve(entries.size());

    for (auto const& entry : entries) {
        if (entry->missed()) {
            result.push_back(entry);
        }
    }

    return result;
}
