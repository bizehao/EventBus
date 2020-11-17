//
// Created by gelldur on 30.10.2019.
//
#pragma once

#include "dexode/eventbus/internal/event_id.hpp"
#include <functional>
#include <type_traits>
#include <memory>

namespace dexode::eventbus {
template <typename>
class Listener;

class Bus;

} // namespace dexode::eventbus

namespace dexode::eventbus::internal {

template <typename EventBus_t>
class ListenerAttorney {
    template <typename>
    friend class dexode::eventbus::Listener;

private:
    static constexpr std::uint32_t newListenerID(std::shared_ptr<EventBus_t>& bus) { return bus->newListenerID(); }

    template <class Event>
    static std::shared_ptr<uint32_t> listen(std::shared_ptr<EventBus_t> bus, const std::uint32_t listenerID,
                                            std::function<void(const Event&)> callback) {
        return bus->template listen<Event>(listenerID, callback);
    }

    static constexpr void unlistenAll(EventBus_t& bus, const std::uint32_t listenerID) { bus.unlistenAll(listenerID); }

    static constexpr void unlisten(std::shared_ptr<EventBus_t>& bus, const std::uint32_t listenerID, std::uint32_t id,
                                   const event_id_t eventID) {
        bus->unlisten(listenerID, id, eventID);
    }
};

} // namespace dexode::eventbus::internal
