#include "GlobalStep.h"

#include <algorithm>

GlobalStep& GlobalStep::get()
{
    static GlobalStep instance;
    return instance;
}

void GlobalStep::setStep (float newStep)
{
    const float prev = step.exchange (newStep, std::memory_order_acq_rel);
    if (prev == newStep)
        return;

    std::vector<Callback> snapshot;
    {
        std::lock_guard<std::mutex> lock (mutex);
        snapshot.reserve (listeners.size());
        for (auto& entry : listeners)
            snapshot.push_back (entry.second);
    }

    for (auto& cb : snapshot)
        cb (newStep);
}

GlobalStep::ListenerId GlobalStep::addListener (Callback cb)
{
    std::lock_guard<std::mutex> lock (mutex);
    const ListenerId id = nextId++;
    listeners.emplace_back (id, std::move (cb));
    return id;
}

void GlobalStep::removeListener (ListenerId id)
{
    std::lock_guard<std::mutex> lock (mutex);
    listeners.erase (
        std::remove_if (listeners.begin(), listeners.end(),
                        [id] (const auto& entry) { return entry.first == id; }),
        listeners.end());
}
