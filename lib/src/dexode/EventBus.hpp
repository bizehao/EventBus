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

namespace dexode
{

class EventBus : public dexode::eventbus::Bus
{
	template <typename>
	friend class dexode::eventbus::internal::ListenerAttorney;

public:
	std::size_t process() override
	{
		return processLimit(std::numeric_limits<std::size_t>::max());
	}

	std::size_t processLimit(std::size_t limit);

protected:
	eventbus::stream::EventStream* obtainStream(
		eventbus::internal::event_id_t eventID,
		eventbus::CreateStreamCallback createStreamCallback);

	bool postponeEvent(eventbus::PostponeHelper& postponeCall) override;
	eventbus::stream::EventStream* findStream(eventbus::internal::event_id_t eventID) const;

	void unlistenAll(std::uint32_t listenerID) override;
	eventbus::stream::EventStream* listen(
		std::uint32_t listenerID,
		eventbus::internal::event_id_t eventID,
		eventbus::CreateStreamCallback createStreamCallback) override;
	void unlisten(std::uint32_t listenerID, eventbus::internal::event_id_t eventID) override;

private:
	mutable std::shared_mutex _mutexStreams;
	std::shared_mutex _mutexProcess;
	std::vector<std::unique_ptr<eventbus::stream::EventStream>> _eventStreams;
	std::map<eventbus::internal::event_id_t, eventbus::stream::EventStream*> _eventToStream;

	eventbus::stream::EventStream* findStreamUnsafe(eventbus::internal::event_id_t eventID) const;
};

} // namespace dexode

namespace dexode
{

inline std::size_t EventBus::processLimit(const std::size_t limit)
{
	std::size_t processCount{0};
	std::lock_guard writeGuardProcess{_mutexProcess}; // Only one process at the time

	std::vector<std::unique_ptr<eventbus::stream::EventStream>> eventStreams;
	{
		std::lock_guard writeGuard{_mutexStreams};
		std::swap(eventStreams, _eventStreams); // move data FROM member
	}

	// Now if any setStream would be called it doesn't conflict without our process call
	for(auto& eventStream : eventStreams)
	{
		const auto runProcessCount = eventStream->process(limit);
		processCount += runProcessCount;
		if(processCount >= limit)
		{
			break;
		}
	}

	{
		std::lock_guard writeGuard{_mutexStreams};
		if(not _eventStreams.empty())
		{
			// If anything was added then we need to add those elements
			std::move(_eventStreams.begin(), _eventStreams.end(), std::back_inserter(eventStreams));
		}
		std::swap(eventStreams, _eventStreams); // move data TO member

		// Check do we need remove something
		if(_eventStreams.size() != _eventToStream.size())
		{
			auto removeFrom = std::remove_if(
				_eventStreams.begin(), _eventStreams.end(), [this](const auto& eventStream) {
					for(const auto& element : _eventToStream)
					{
						// Don't remove if we point to the same place (is it UB ?)
						if(element.second == eventStream.get())
						{
							return false;
						}
					}
					return true;
				});
			assert(removeFrom != _eventStreams.end());
            _eventStreams.erase(removeFrom, _eventStreams.end());
		}
	}

	return processCount;
}

inline eventbus::stream::EventStream* EventBus::findStream(
	const eventbus::internal::event_id_t eventID) const
{
	std::shared_lock readGuard{_mutexStreams};
	return findStreamUnsafe(eventID);
}

inline void EventBus::unlistenAll(const std::uint32_t listenerID)
{
	std::shared_lock readGuard{_mutexStreams};
	for(auto& eventStream : _eventToStream)
	{
		eventStream.second->removeListener(listenerID);
	}
}

inline eventbus::stream::EventStream* EventBus::findStreamUnsafe(
	const eventbus::internal::event_id_t eventID) const
{
	auto lookup = _eventToStream.find(eventID);
	return lookup != _eventToStream.end() ? lookup->second : nullptr;
}

inline eventbus::stream::EventStream* EventBus::obtainStream(
	const eventbus::internal::event_id_t eventID,
	eventbus::CreateStreamCallback createStreamCallback)
{
	std::lock_guard writeGuard{_mutexStreams};
	auto* found = findStreamUnsafe(eventID);
	if(found != nullptr)
	{
		return found;
	}
	else
	{
		auto stream = createStreamCallback();
		_eventStreams.push_back(std::move(stream));
		_eventToStream[eventID] = _eventStreams.back().get();
		return _eventStreams.back().get();
	}
}

inline bool EventBus::postponeEvent(eventbus::PostponeHelper& postponeCall)
{
	auto* eventStream = obtainStream(postponeCall.eventID, postponeCall.createStreamCallback);
	eventStream->postpone(std::move(postponeCall.event));
	return true;
}

inline eventbus::stream::EventStream* EventBus::listen(const std::uint32_t,
												const eventbus::internal::event_id_t eventID,
												eventbus::CreateStreamCallback createStreamCallback)
{
	auto* eventStream = obtainStream(eventID, createStreamCallback);
	return eventStream;
}

inline void EventBus::unlisten(const std::uint32_t listenerID,
						const eventbus::internal::event_id_t eventID)
{
	auto* eventStream = findStream(eventID);
	if(eventStream != nullptr)
	{
		eventStream->removeListener(listenerID);
	}
}

} // namespace dexode
