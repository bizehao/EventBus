//
// Created by gelldur on 26.11.2019.
//
#pragma once

#include <any>
#include <atomic>
#include <memory>

#include "dexode/eventbus/Listener.hpp"
#include "dexode/eventbus/internal/event_id.hpp"
#include "dexode/eventbus/stream/ProtectedEventStream.hpp"

namespace dexode::eventbus {
namespace internal {
template <typename EventBus_t>
class ListenerAttorney;
}
class Bus;

template <typename Event>
using DefaultEventStream = eventbus::stream::ProtectedEventStream<Event>;
using CreateStreamCallback = std::shared_ptr<eventbus::stream::EventStream> (*const)();
using PostponeCallback = bool (*const)(Bus& bus, std::any event);

template <typename Event>
bool postpone(Bus& bus, std::any event) {
    return bus.postpone(std::move(std::any_cast<Event>(event)));
}

template <typename Event>
std::shared_ptr<eventbus::stream::EventStream> createDefaultEventStream() {
    return std::make_shared<DefaultEventStream<Event>>();
}

class PostponeHelper {
public:
    internal::event_id_t eventID = nullptr;
    std::any event;

    PostponeCallback postponeCallback = nullptr;         // function pointer
    CreateStreamCallback createStreamCallback = nullptr; // function pointer

    PostponeHelper(const internal::event_id_t eventId, std::any&& event, PostponeCallback postponeCallback,
                   CreateStreamCallback createStreamCallback)
        : eventID(eventId), event(std::forward<std::any>(event)), postponeCallback(postponeCallback),
          createStreamCallback(createStreamCallback) {}

    template <typename Event>
    static std::shared_ptr<PostponeHelper> create(std::any&& event) {
        return std::make_shared<PostponeHelper>(internal::event_id<Event>(), std::forward<std::any>(event), postpone<Event>,
                                                createDefaultEventStream<Event>);
    }
};

class Bus {
    template <typename>
    friend class dexode::eventbus::internal::ListenerAttorney;

public:
    using Listener = eventbus::Listener<dexode::eventbus::Bus>;

    Bus() = default;
    virtual ~Bus() = default;

    virtual std::size_t process() = 0;
    virtual std::size_t processLimit(std::size_t limit) = 0;

    template <typename Event>
    bool postpone(Event event) {
        static_assert(internal::validateEvent<Event>(), "Invalid event");
        auto postponeCall = PostponeHelper::create<Event>(std::move(event));
        return postponeEvent(postponeCall);
    }

protected:
    virtual bool postponeEvent(std::shared_ptr<eventbus::PostponeHelper> postponeCall) = 0;
    virtual std::shared_ptr<eventbus::stream::EventStream> listen_impl(std::uint32_t listenerID, internal::event_id_t eventID,
                                                                       CreateStreamCallback createStreamCallback) = 0;

    virtual void unlistenAll(std::uint32_t listenerID) = 0;
    virtual void unlisten(std::uint32_t listenerID, std::uint32_t id, internal::event_id_t eventID) = 0;

private:
    std::atomic<std::uint32_t> _lastID{0};

    std::uint32_t newListenerID() {
        return ++_lastID; // used for generate unique listeners ID's
    }

    template <class Event>
    std::shared_ptr<uint32_t> listen(const std::uint32_t listenerID, std::function<void(const Event&)> callback) {
        static_assert(internal::validateEvent<Event>(), "Invalid event");
        assert(callback && "callback should be valid"); // Check for valid object

        constexpr auto eventID = internal::event_id<Event>();

        auto eventStream = listen_impl(listenerID, eventID, createDefaultEventStream<Event>);
        return eventStream->addListener(std::function<void(const Event&)>(callback));
    }
};

} // namespace dexode::eventbus
