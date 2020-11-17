//
// Created by gelldur on 23.12.2019.
//
#pragma once

#include <memory>
#include <vector>

#include "Perk.hpp"
#include "dexode/EventBus.hpp"
#include "WaitPerk.hpp"
#include "TagPerk.hpp"
#include "PassPerk.hpp"

namespace dexode::eventbus::perk {

class PerkEventBus : public EventBus {
private:
    class RegisterHelper {
        friend PerkEventBus;

    public:
        template <typename Perk_t>
        RegisterHelper& registerPrePostpone(perk::Flag (Perk_t::*method)(PostponeHelper&)) {
            _bus->_onPrePostpone.push_back(std::bind(method, static_cast<Perk_t*>(_perk), std::placeholders::_1));
            return *this;
        }

        template <typename Perk_t>
        RegisterHelper& registerPostPostpone(perk::Flag (Perk_t::*method)(PostponeHelper&)) {
            _bus->_onPostPostpone.push_back(std::bind(method, static_cast<Perk_t*>(_perk), std::placeholders::_1));
            return *this;
        }

    private:
        PerkEventBus* _bus;
        Perk* _perk;

        RegisterHelper(PerkEventBus* bus, Perk* perk) : _bus(bus), _perk(perk) {}
    };

    RegisterHelper addPerk(std::shared_ptr<Perk> perk);

public:
    std::shared_ptr<dexode::eventbus::perk::WaitPerk> join_wait_perk() {
        auto ptr = std::make_shared<dexode::eventbus::perk::WaitPerk>();
        addPerk(ptr).registerPostPostpone(&dexode::eventbus::perk::WaitPerk::onPostponeEvent);
        return ptr;
    }

    void join_pass_perk(std::shared_ptr<dexode::eventbus::Bus> bus) {
        auto ptr = std::make_shared<dexode::eventbus::perk::PassEverythingPerk>(bus);
        addPerk(std::make_unique<dexode::eventbus::perk::PassEverythingPerk>(bus))
            .registerPrePostpone(&dexode::eventbus::perk::PassEverythingPerk::onPrePostponeEvent);
    }

    template <template <typename> typename T>
    std::shared_ptr<dexode::eventbus::perk::TagPerk<T>> join_tag_perk(std::shared_ptr<dexode::eventbus::Bus> bus,
                                                                      std::string key) {
        auto ptr = std::make_shared<dexode::eventbus::perk::TagPerk<T>>(key, bus);
        addPerk(ptr).registerPrePostpone(&dexode::eventbus::perk::TagPerk<T>::onPrePostponeEvent);
        return ptr;
    }

    //	template <typename T>
    //	T* getPerk()
    //	{
    //		auto found =
    //			std::find_if(_perks.begin(), _perks.end(), [](const std::shared_ptr<Perk>& perk) {
    //				return dynamic_cast<T*>(perk.get()) != nullptr;
    //			});
    //		if(found != _perks.end())
    //		{
    //			return static_cast<T*>(found->get());
    //		}
    //		return nullptr;
    //	}

protected:
    bool postponeEvent(std::shared_ptr<eventbus::PostponeHelper> postponeCall) override;

private:
    std::vector<std::shared_ptr<Perk>> _perks;
    std::vector<std::function<perk::Flag(PostponeHelper&)>> _onPrePostpone;
    std::vector<std::function<perk::Flag(PostponeHelper&)>> _onPostPostpone;
};

} // namespace dexode::eventbus::perk
