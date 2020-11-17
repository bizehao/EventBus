#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <iostream>
#include "dexode/eventbus/internal/ListenerAttorney.hpp"
#include "dexode/eventbus/internal/event_id.hpp"
#include "dexode/eventbus/internal/listener_traits.hpp"

namespace dexode::eventbus {

template <class Bus>
class Listener {
public:
    explicit Listener(std::shared_ptr<Bus> bus)
        : _id{internal::ListenerAttorney<Bus>::newListenerID(bus)}, _bus{std::move(bus)} {}

    Listener(const Listener& other) = delete;
    // To see why move is disabled search for tag: FORBID_MOVE_LISTENER in tests
    // Long story short, what if we capture 'this' in lambda during registering listener in ctor
    Listener(Listener&& other) = delete;

    ~Listener() {
        if (_bus != nullptr) // could be moved
        {
            unlistenAll();
        }
    }

    Listener& operator=(const Listener& other) = delete;
    // To see why move is disabled search for tag: FORBID_MOVE_LISTENER in tests
    Listener& operator=(Listener&& other) = delete;

    template <class Event, typename _ = void>
    constexpr std::shared_ptr<uint32_t> listen(std::function<void(const Event&)> callback) {
        static_assert(internal::validateEvent<Event>(), "Invalid event");
        return listenToCallback<Event>(std::move(callback));
    }

    template <class EventCallback, typename Event = internal::first_argument<EventCallback>>
    constexpr std::shared_ptr<uint32_t> listen(EventCallback&& callback) {
        static_assert(std::is_const_v<std::remove_reference_t<Event>>, "Event should be const");
        static_assert(std::is_reference_v<Event>, "Event should be const & (reference)");
        using PureEvent = std::remove_const_t<std::remove_reference_t<Event>>;
        static_assert(internal::validateEvent<PureEvent>(), "Invalid event");
        listenToCallback<PureEvent>(std::forward<EventCallback>(callback));
    }

    void unlistenAll() {
        if (_bus == nullptr) {
            throw std::runtime_error{"bus is null"};
        }
        internal::ListenerAttorney<Bus>::unlistenAll(*_bus, _id);
    }

    template <typename Event>
    void unlisten(uint32_t id = 0) {
        static_assert(internal::validateEvent<Event>(), "Invalid event");
        if (_bus == nullptr) {
            throw std::runtime_error{"bus is null"};
        }
        internal::ListenerAttorney<Bus>::unlisten(_bus, _id, id, internal::event_id<Event>());
    }

    // We want more explicit move so user knows what is going on
    void transfer(Listener&& from) {
        if (this == &from) {
            throw std::runtime_error("Self transfer not allowed");
        }

        if (_bus != nullptr) {
            unlistenAll(); // remove previous
        }
        // we don't have to reset listener ID as bus is moved and we won't call unlistenAll
        _id = from._id;
        _bus = std::move(from._bus);
    }

    const std::shared_ptr<Bus>& getBus() const { return _bus; }

private:
    template <class Event>
    std::shared_ptr<uint32_t> listenToCallback(std::function<void(const Event&)> callback) {
        static_assert(internal::validateEvent<Event>(), "Invalid event");
        if (_bus == nullptr) {
            throw std::runtime_error{"bus is null"};
        }
        return internal::ListenerAttorney<Bus>::template listen<Event>(_bus, _id, callback);
    }

    std::uint32_t _id;
    std::shared_ptr<Bus> _bus;
};

} // namespace dexode::eventbus
