//
// Created by gelldur on 24.12.2019.
//
#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "Perk.hpp"

namespace dexode::eventbus {
class PostponeHelper;
}

namespace dexode::eventbus::perk {

class WaitPerk : public Perk {
public:
    /**
	 * @return true when events are waiting in bus
     */
    bool wait();

    /**
	 * @param timeout
     * @return true when events are waiting in bu
     */
    template<typename Rep, typename Period>
    bool waitFor(std::chrono::duration<Rep, Period> timeout) {
        using namespace std::chrono_literals;
        std::unique_lock<std::mutex> lock(_waitMutex);
        if (_hasEvents) {
            _hasEvents = false; // reset
            return true;
        }
        if (_eventWaiting.wait_for(lock, timeout, [this]() { return _hasEvents; })) {
            // At this moment we are still under mutex
            _hasEvents = false; // reset
            return true;
        }
        return false;
    }

    Flag onPostponeEvent(PostponeHelper& postponeCall);

private:
    std::condition_variable _eventWaiting;
    std::mutex _waitMutex;
    bool _hasEvents = false;
};

} // namespace dexode::eventbus::perk

