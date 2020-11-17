//
// Created by gelldur on 26.11.2019.
//
#pragma once

#include <any>
#include <limits>
#include <map>
#include <memory>
#include <shared_mutex>

#include "dexode/eventbus/Bus.hpp"

namespace dexode {

class EventBus : public dexode::eventbus::Bus {
    template <typename>
    friend class dexode::eventbus::internal::ListenerAttorney;

public:
    std::size_t process() override { return processLimit(std::numeric_limits<std::size_t>::max()); }

    std::size_t processLimit(std::size_t limit) override;

protected:
    std::shared_ptr<eventbus::stream::EventStream> obtainStream(eventbus::internal::event_id_t eventID,
                                                                eventbus::CreateStreamCallback createStreamCallback);

    bool postponeEvent(std::shared_ptr<eventbus::PostponeHelper> postponeCall) override;
    //检查当前事件流是否存在
    std::shared_ptr<eventbus::stream::EventStream> findStream(eventbus::internal::event_id_t eventID) const;

    void unlistenAll(std::uint32_t listenerID) override;
    std::shared_ptr<eventbus::stream::EventStream> listen_impl(std::uint32_t listenerID, eventbus::internal::event_id_t eventID,
                                                               eventbus::CreateStreamCallback createStreamCallback) override;
    void unlisten(std::uint32_t listenerID, std::uint32_t id, eventbus::internal::event_id_t eventID) override;

private:
    mutable std::shared_mutex _mutexStreams;
    std::shared_mutex _mutexProcess;
    std::vector<std::shared_ptr<eventbus::PostponeHelper>> _eventStreams;
    std::map<eventbus::internal::event_id_t, std::shared_ptr<eventbus::stream::EventStream>> _eventToStream;
};

} // namespace dexode
