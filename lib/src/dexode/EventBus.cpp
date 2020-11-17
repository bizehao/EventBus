#include "EventBus.hpp"

namespace dexode {

std::size_t EventBus::processLimit(const std::size_t limit) {
    std::size_t processCount{0};
    std::lock_guard writeGuardProcess{_mutexProcess}; // Only one process at the time

    std::vector<std::shared_ptr<eventbus::PostponeHelper>> eventStreams;
    {
        std::lock_guard writeGuard{_mutexStreams};
        std::swap(eventStreams, _eventStreams); // move data FROM member
    }

    std::vector<std::shared_ptr<eventbus::stream::EventStream>> vec_event;
    // Now if any setStream would be called it doesn't conflict without our process call
    for (auto& es : eventStreams) {
        auto v = _eventToStream[es->eventID];
        v->postpone(std::move(es->event));
        if (std::find(vec_event.begin(), vec_event.end(), v) == vec_event.end()) {
            vec_event.push_back(v);
        }
        processCount++;
        if (processCount >= limit) {
            break;
        }
    }

    for (auto v : vec_event) {
        v->process();
    }

    eventStreams.erase(eventStreams.begin(), std::next(eventStreams.begin(), processCount));

    {
        std::lock_guard writeGuard{_mutexStreams};
        if (!_eventStreams.empty()) {
            // If anything was added then we need to add those elements
            std::move(_eventStreams.begin(), _eventStreams.end(), std::back_inserter(eventStreams));
        }
        std::swap(eventStreams, _eventStreams); // move data TO
    }

    return processCount;
}

std::shared_ptr<eventbus::stream::EventStream> EventBus::findStream(const eventbus::internal::event_id_t eventID) const {
    auto lookup = _eventToStream.find(eventID);
    return lookup != _eventToStream.end() ? lookup->second : nullptr;
}

void EventBus::unlistenAll(const std::uint32_t listenerID) {
    std::shared_lock readGuard{_mutexStreams};
    for (auto& eventStream : _eventToStream) {
        eventStream.second->removeListener(listenerID);
    }
}

std::shared_ptr<eventbus::stream::EventStream> EventBus::obtainStream(const eventbus::internal::event_id_t eventID,
                                                                      eventbus::CreateStreamCallback createStreamCallback) {

    auto found = findStream(eventID);
    if (found != nullptr) {
        return found;
    } else {
        auto stream = createStreamCallback();
        _eventToStream[eventID] = stream;
        return stream;
    }
}

bool EventBus::postponeEvent(std::shared_ptr<eventbus::PostponeHelper> postponeCall) {
    std::lock_guard writeGuard{_mutexStreams};
    auto v = findStream(postponeCall->eventID);
    if (v == nullptr) {
        throw std::runtime_error{"没有这样的事件"};
    }
    _eventStreams.push_back(postponeCall);
    return true;
}

std::shared_ptr<eventbus::stream::EventStream> EventBus::listen_impl(const std::uint32_t listenerID,
                                                                     const eventbus::internal::event_id_t eventID,
                                                                     eventbus::CreateStreamCallback createStreamCallback) {
    std::lock_guard writeGuard{_mutexStreams};
    auto eventStream = obtainStream(eventID, createStreamCallback);
    return eventStream;
}

void EventBus::unlisten(const std::uint32_t listenerID, std::uint32_t id, const eventbus::internal::event_id_t eventID) {
    auto eventStream = findStream(eventID);
    if (eventStream != nullptr) {
        eventStream->removeListener(id);
        std::cout << "count: " << eventStream->listener_count() << std::endl;
        if (eventStream->listener_count() == 0) {
            auto itero = _eventToStream.find(eventID);
            if (itero != _eventToStream.end()) {
                _eventToStream.erase(itero);
            }
        }
    }
}

} // namespace dexode
