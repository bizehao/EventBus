//
// Created by gelldur on 22.12.2019.
//
#include "Gui.hpp"

#include <iostream>
#include <sync_print.h>
#include "event.hpp"
#include <thread>

Gui::Gui(const std::shared_ptr<EventBus>& bus) : _listener{bus} {
    _listener.listen([this](const event::NewTeamMember& event) {
        sync_print{} << "Gui NewTeamMember thread id: " << std::this_thread::get_id() << std::endl;
        sync_print{} << "v: " << event.memberName << std::endl;
        _sackOfGold.emplace(event.memberName, 0);
    });

    _listener.listen([this](const event::TagEvent<event::GoldUpdate>& event) {
        sync_print{} << "Gui GoldUpdate thread id: " << std::this_thread::get_id() << std::endl;
        auto found = _sackOfGold.find(event.tag);
        if (found != _sackOfGold.end()) {
            found->second = event.data.goldCount;
        }
    });

    _listener.listen([this](const event::TagEvent<event::Test_Data>& event) {
        sync_print{} << "Gui Test_Data thread id: " << std::this_thread::get_id() << std::endl;
        auto found = _sackOfGold.find(event.tag);
        if (found != _sackOfGold.end()) {
            found->second = event.data.haha;
        }
    });

    _listener.listen([](const event::Null_Data& event) {
        sync_print{} << "Gui single Null_Data thread id: " << std::this_thread::get_id() << std::endl;
        sync_print{} << "it is of my thinking " << event.vv << std::endl;
    });


}

void Gui::draw() {
    sync_print{} << "Gui draw thread id: " << std::this_thread::get_id() << std::endl;
    std::cout << "-----------------------------\n";
    for (const auto& player : _sackOfGold) {
        std::cout << "Name: " << player.first << " - gold: " << player.second << "\n";
    }
    std::cout << "-----------------------------" << std::endl;
}
