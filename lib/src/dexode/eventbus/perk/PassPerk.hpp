//
// Created by gelldur on 24.12.2019.
//
#pragma once

#include <memory>

#include "Perk.hpp"

namespace dexode::eventbus
{
class PostponeHelper;
class Bus;
} // namespace dexode::eventbus

namespace dexode::eventbus::perk
{

class PassEverythingPerk : public Perk
{
public:
	PassEverythingPerk(std::shared_ptr<dexode::eventbus::Bus> passTo)
		: _passToBus{std::move(passTo)}
	{}

	Flag onPrePostponeEvent(PostponeHelper& postponeCall);

private:
	std::shared_ptr<dexode::eventbus::Bus> _passToBus;
};

} // namespace dexode::eventbus::perk

namespace dexode::eventbus::perk
{

inline Flag PassEverythingPerk::onPrePostponeEvent(PostponeHelper& postponeCall)
{
	postponeCall.postponeCallback(*_passToBus, std::move(postponeCall.event));
	return Flag::postpone_cancel;
}

} // namespace dexode::eventbus::perk
