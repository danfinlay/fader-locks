#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

// Process-wide registry that mirrors the global step value across every
// plugin instance loaded in the host. The knob in any Fader Locks instance
// writes here; all other instances read and follow.
class GlobalStep
{
public:
    using ListenerId = int;
    using Callback   = std::function<void (float)>;

    static GlobalStep& get();

    float getStep() const noexcept { return step.load (std::memory_order_acquire); }

    // Stores the new value and notifies every listener. Listeners that wrote
    // the value themselves should compare against their own current state to
    // avoid feedback loops.
    void setStep (float newStep);

    ListenerId addListener (Callback cb);
    void       removeListener (ListenerId id);

private:
    GlobalStep() = default;

    std::atomic<float> step { 0.0f };

    std::mutex                                          mutex;
    ListenerId                                          nextId { 1 };
    std::vector<std::pair<ListenerId, Callback>>        listeners;
};
