#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include "event.hpp"
#include "dexode/EventBus.hpp"
#include "dexode/eventbus/perk/PerkEventBus.hpp"
#include "dexode/eventbus/perk/WaitPerk.hpp"
#include "dexode/eventbus/perk/TagPerk.hpp"
#include <sync_print.h>

using namespace std::chrono_literals;

namespace {
struct EventTest {
    std::string data;
    std::chrono::steady_clock::time_point created = std::chrono::steady_clock::now();
};
} // namespace

//int main() {
//    sync_print{} << "main thread id: " << std::this_thread::get_id() << std::endl;

//    auto bus = std::make_shared<dexode::eventbus::perk::PerkEventBus>();

//    auto p = bus->join_tag_perk<event::TagEvent>(bus,"test");
//    p->wrapTag<event::TagEvent<event::GoldUpdate>>();
//    auto waitPerk = bus->join_wait_perk();

//    dexode::eventbus::perk::PerkEventBus::Listener listener{bus};
//    listener.listen([bus](const event::TagEvent<event::GoldUpdate>& event) {

//        sync_print{} << "listener thread id: " << std::this_thread::get_id() << std::endl;

//        const auto eventAge = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - event.data.time);

//        std::cout << "Event:" << event.tag << " old: " << eventAge.count() << "ms" << std::endl;
//    });

//    std::atomic<bool> isWorking = true;

//    std::vector<std::thread> producers;
//    // Worker which will send event every 500 ms
//    producers.emplace_back([&bus, &isWorking]() {
//        while (isWorking) {
//            auto time = std::chrono::steady_clock::now();
//            std::this_thread::sleep_for(10ms);
//            bus->postpone(event::GoldUpdate{std::move(time)});
//        }
//    });

//    while (waitPerk->wait()) {
//        bus->process();
//    }
////    for (int i = 0; i < 20;) {
////        auto start = std::chrono::steady_clock::now();
////        if (waitPerk->waitFor(2s)) {
////            const auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

////            std::cout << "[SUCCESS] I was sleeping for: " << sleepTime.count() << " ms i:" << i << std::endl;
////            i += bus->process();
////        } else {
////            const auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
////            // No events waiting for us
////            std::cout << "I was sleeping for: " << sleepTime.count() << " ms" << std::endl;
////        }
////    }

//    //isWorking = false;
//    for (auto& producer : producers) {
//        producer.join();
//    }
//}

//TEST_CASE("Should wait for event being scheduled", "[concurrent][EventBus]")
//{
//    auto bus = std::make_shared<dexode::eventbus::perk::PerkEventBus>();
//    bus->addPerk(std::make_unique<dexode::eventbus::perk::WaitPerk>())
//        .registerPostPostpone(&dexode::eventbus::perk::WaitPerk::onPostponeEvent);

//    auto* waitPerk = bus->getPerk<dexode::eventbus::perk::WaitPerk>();
//    REQUIRE(waitPerk != nullptr);

//    dexode::eventbus::perk::PerkEventBus::Listener listener{bus};
//    listener.listen([bus](const EventTest& event) {
//        const auto eventAge = std::chrono::duration_cast<std::chrono::milliseconds>(
//            std::chrono::steady_clock::now() - event.created);
//        CHECK(eventAge < 5ms);
//        std::cout << "Event:" << event.data << " old: " << eventAge.count() << "ms" << std::endl;
//    });

//    std::atomic<bool> isWorking = true;

//    std::vector<std::thread> producers;
//    producers.emplace_back([&bus, &isWorking]() {
//        while(isWorking)
//        {
//            std::this_thread::sleep_for(10ms);
//            bus->postpone(EventTest{"producer1"});
//        }
//    });

//    for(int i = 0; i < 20;)
//    {
//        auto start = std::chrono::steady_clock::now();
//        if(waitPerk->waitFor(40ms))
//        {
//            const auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(
//                std::chrono::steady_clock::now() - start);
//            CHECK(sleepTime >= 9ms);

//            std::cout << "[SUCCESS] I was sleeping for: " << sleepTime.count() << " ms i:" << i
//                      << std::endl;
//            i += bus->process();
//        }
//        else
//        {
//            const auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(
//                std::chrono::steady_clock::now() - start);
//            CHECK(sleepTime < 5ms);
//            // No events waiting for us
//            std::cout << "I was sleeping for: " << sleepTime.count() << " ms" << std::endl;
//        }
//    }

//    isWorking = false;
//    for(auto& producer : producers)
//    {
//        producer.join();
//    }
//}

class AA {
public:
    void sayAA() { std::cout << "AA " << v << std::endl; }

    template<typename T>
    void sayComm(T t) {
        std::cout << "comm " << t << std::endl;
    }

public:
    int v = 500;
};

class BB : public AA {
public:
    void sayBB() { std::cout << "BB " << v << std::endl; }

public:
    int v = 888;
};

template<typename T,typename S>
void test(std::shared_ptr<T> t,S s) {
    t->template sayComm(s);
}

int main() {
    auto bb = std::make_shared<BB>();
    //    bb.sayBB();
    //    AA& aa = static_cast<AA&>(bb);
    //    aa.sayAA();
    //    aa.v = 999;
    //    aa.sayAA();
    //    BB& cc = static_cast<BB&>(aa);
    //    cc.sayBB();
    //    cc.sayAA();
    //    cc.AA::v = 589;
    //    cc.sayAA();
    //bb.sayComm(55);

    test(bb, 88);

    auto bus = std::make_shared<dexode::EventBus>();


    dexode::eventbus::Listener listener{bus};
    std::cout << "111" << std::endl;
    //dexode::EventBus::Listener listener{bus};
    listener.listen([](const EventTest& test) { std::cout << test.data << std::endl; });

    bus->postpone(EventTest{"haha1", std::chrono::steady_clock::now()});
    bus->postpone(EventTest{"haha2", std::chrono::steady_clock::now()});
    bus->postpone(EventTest{"haha3", std::chrono::steady_clock::now()});
    bus->postpone(EventTest{"haha4", std::chrono::steady_clock::now()});

    bus->processLimit(3);
    std::this_thread::sleep_for(5s);
    bus->postpone(EventTest{"haha5", std::chrono::steady_clock::now()});
    bus->postpone(EventTest{"haha6", std::chrono::steady_clock::now()});
    bus->process();

    while (true) {
    }
}
