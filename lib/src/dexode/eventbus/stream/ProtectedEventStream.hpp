#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>
#include <shared_mutex>
#include <map>
#include "dexode/eventbus/stream/EventStream.hpp"
#include <random>

namespace dexode::eventbus::stream {

template <typename Event, typename CallbackReturn = void, typename... ExtraArgTypes>
class ProtectedEventStream : public EventStream {
    using Callback = std::function<CallbackReturn(const Event&, ExtraArgTypes...)>;

public:
    void postpone(std::any event) override {
        auto myEvent = std::any_cast<Event>(event);
        std::lock_guard writeGuard{_mutexEvent};
        _queue.push_back(std::move(myEvent));
    }

    std::size_t process(const std::size_t limit) override {
        std::vector<Event> processEvents;
        {
            std::lock_guard writeGuard{_mutexEvent};
            if (limit >= _queue.size()) {
                processEvents.reserve(_queue.size());
                std::swap(processEvents, _queue);
            } else {
                const auto countElements = std::min(limit, _queue.size());
                processEvents.reserve(countElements);
                std::move(_queue.begin(), std::next(_queue.begin(), countElements), std::back_inserter(processEvents));
                _queue.erase(_queue.begin(), std::next(_queue.begin(), countElements));
            }
        }

        for (const auto& event : processEvents) {
            // At this point we need to consider transaction safety as during some notification
            // we can add/remove listeners
            _isProcessing = true;
            for (auto& iten : _callbacks) {
                iten.second(event);
            }
            _isProcessing = false;

            flushWaitingOnes();
        }

        return processEvents.size();
    }

    std::shared_ptr<uint32_t> addListener(std::any callback) override {
        std::lock_guard writeGuard{_mutexCallbacks};
        auto myCallback = std::any_cast<Callback>(callback);
        std::shared_ptr<uint32_t> key;
        if (_isProcessing) {
            key = std::make_shared<uint32_t>(0);
            waiting_add_.push_back({key, std::move(myCallback)});
        } else {
            key = std::make_shared<uint32_t>(rawAddListener(std::move(myCallback)));
        }
        return key;
    }

    bool removeListener(const std::uint32_t listenerID) override {
        std::lock_guard writeGuard{_mutexCallbacks};
        if (_isProcessing) {
            waiting_remove_.push_back(listenerID);
            return true;
        }

        return rawRemoveListener(listenerID);
    }

    [[nodiscard]] bool hasEvents() const {
        std::shared_lock readGuard{_mutexEvent};
        return !_queue.empty();
    }

    uint32_t listener_count() override { return std::distance(_callbacks.begin(), _callbacks.end()); };

private:
    //处理刚刚要等待的任务
    void flushWaitingOnes() {
        std::lock_guard writeGuard{_mutexCallbacks};
        if (!waiting_remove_.empty()) {
            for (auto& element : waiting_remove_) {
                rawRemoveListener(element);
            }
            waiting_remove_.clear();
        }
        if (!waiting_add_.empty()) {
            for (auto& element : waiting_add_) {
                *(element.first) = rawAddListener(element.second);
            }
            waiting_add_.clear();
        }
    }

    //返回 listener id
    uint32_t rawAddListener(Callback callback) {
        std::random_device rd;
        uint32_t id = 0;
        do {
            std::default_random_engine dre{rd()};
            id = dre();
        } while (id == 0);

        _callbacks[id] = std::forward<Callback>(callback);

        return id;
    }

    //根据id删除监听器
    bool rawRemoveListener(const std::uint32_t listenerID) {
        if (listenerID == 0) {
            _callbacks.clear();
        } else {
            auto found = _callbacks.find(listenerID);
            if (found == _callbacks.end()) {
                return false;
            }
            _callbacks.erase(found);
        }

        return true;
    }

    std::vector<Event> _queue;
    std::map<uint32_t, Callback> _callbacks;

    std::atomic<bool> _isProcessing{false};
    std::vector<std::pair<std::shared_ptr<uint32_t>, Callback>> waiting_add_;
    std::vector<uint32_t> waiting_remove_;

    std::shared_mutex _mutexEvent;
    std::shared_mutex _mutexCallbacks;
};

} // namespace dexode::eventbus::stream
