#include "WaitPerk.hpp"

namespace dexode::eventbus::perk
{

bool WaitPerk::wait()
{
	using namespace std::chrono_literals;
	std::unique_lock<std::mutex> lock(_waitMutex);
	if(_hasEvents)
	{
		_hasEvents = false; // reset, assume that processing of events took place
		return true;
	}
	_eventWaiting.wait(lock, [this]() { return _hasEvents; });

	// At this moment we are still under mutex
	_hasEvents = false; // reset, assume that processing of events took place
	return true;
}

Flag WaitPerk::onPostponeEvent(PostponeHelper&)
{
	{
		std::lock_guard<std::mutex> lock(_waitMutex);
		_hasEvents = true;
	}
	_eventWaiting.notify_one();
	return Flag::postpone_continue;
}

} // namespace dexode::eventbus::perk
