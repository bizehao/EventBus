//
// Created by gelldur on 22.12.2019.
//
#pragma once
#include <string>
#include <chrono>

namespace event {

struct Null_Data {
    std::string vv;
};

struct Test_Data {
    int goldCount;
    double haha;
};

struct GoldUpdate {
    std::chrono::steady_clock::time_point time;
};

struct NewTeamMember {
    std::string memberName;
};

template<typename T>
struct TagEvent {
    using Event = T;
    std::string tag;
    Event data;
};

} // namespace event
