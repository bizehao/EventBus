//
// Created by gelldur on 24.12.2019.
//
#pragma once

#include <any>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include "Perk.hpp"
#include "dexode/eventbus/Bus.hpp"
#include "dexode/eventbus/internal/event_id.hpp"



namespace dexode::eventbus::perk {

namespace {

template<typename T>
struct tag_type {
    static constexpr bool value = false;
};

template<template<typename> typename A, typename B>
struct tag_type<A<B>> {
    static constexpr bool value = true;
    using type = B;
};

template<template<typename> typename Ori, typename Comp>
constexpr bool is_origin() {
    static_assert(tag_type<Comp>::value, "no suitable 1");
    using type = typename tag_type<Comp>::type;
    static_assert(std::is_same_v<Ori<type>, Comp>, "no suitable 2");
    return true;
};

} // namespace

template<template<typename> typename T>
class TagPerk : public Perk {

public:
    TagPerk(std::string tag, std::shared_ptr<dexode::eventbus::Bus> owner) : _tag{std::move(tag)}, _ownerBus{owner} { }

    Flag onPrePostponeEvent(PostponeHelper& postponeCall){
        if(auto found = _eventsToWrap.find(postponeCall.eventID); found != _eventsToWrap.end())
        {
            found->second(postponeCall.event);
            return Flag::postpone_cancel;
        }

        return Flag::postpone_continue;
    }
    /**
     *  template<typename T>
     *  struct TagEvent {
     *      using Event = T;
     *      std::string tag;
     *      Event data;
     *  };
     */
    template<typename V>
    TagPerk& wrapTag() {
        static_assert(is_origin<T, V>(), "type errror");
        return wrapTag_impl<V>();
    }

private:
    template<typename TagEvent>
    TagPerk& wrapTag_impl() {
        static_assert(internal::validateEvent<TagEvent>(), "Invalid tag event");
        static_assert(internal::validateEvent<typename TagEvent::Event>(), "Invalid event");
        constexpr auto eventID = internal::event_id<typename TagEvent::Event>();

        _eventsToWrap[eventID] = [this](std::any event) {
            TagEvent newEvent{_tag, std::move(std::any_cast<typename TagEvent::Event>(event))};
            _ownerBus->postpone<TagEvent>(std::move(newEvent));
        };
        return *this;
    }
private:
    std::map<internal::event_id_t, std::function<void(std::any)>> _eventsToWrap;
    std::string _tag;
    std::shared_ptr<dexode::eventbus::Bus> _ownerBus;
};

} // namespace dexode::eventbus::perk
