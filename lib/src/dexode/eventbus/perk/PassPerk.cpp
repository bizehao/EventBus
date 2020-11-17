#include "PassPerk.hpp"
#include "dexode/eventbus/Bus.hpp"

namespace dexode::eventbus::perk
{

Flag PassEverythingPerk::onPrePostponeEvent(PostponeHelper& postponeCall)
{
	postponeCall.postponeCallback(*_passToBus, std::move(postponeCall.event));
	return Flag::postpone_cancel;
}

} // namespace dexode::eventbus::perk