/**
 * @brief Sample code to play with!
 */

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include <dexode/EventBus.hpp>

using EventBus = dexode::EventBus;
using Listener = dexode::EventBus::Listener;

// namespace event // Example namespace for events
//{
// struct Gold // Event that will be proceed when our gold changes
//{
//	int value = 0;
//};
//} // namespace event
//
// enum class Monster
//{
//	Frog,
//	Tux,
//	Shark
//};
//
// class Character
//{
// public:
//	Character(const std::shared_ptr<EventBus>& eventBus)
//		: _bus{eventBus}
//	{}
//
//	void kill(Monster monsterType)
//	{
//		if(Monster::Frog == monsterType)
//		{
//			_gold += 1;
//		}
//		else if(Monster::Tux == monsterType)
//		{
//			_gold += 100;
//		}
//		else if(Monster::Shark == monsterType)
//		{
//			_gold += 25;
//		}
//		_bus->postpone(event::Gold{_gold});
//	}
//
// private:
//	int _gold = 0;
//	std::shared_ptr<EventBus> _bus;
//};
//
// class UIWallet
//{
// public:
//	UIWallet(const std::shared_ptr<EventBus>& eventBus)
//		: _listener{eventBus}
//	{}
//
//	void onDraw() // Example "draw" of UI
//	{
//		std::cout << "Gold:" << _gold << std::endl;
//	}
//
//	void onEnter() // We could also do such things in ctor and dtor but a lot of UI has something
//				   // like this
//	{
//		_listener.listen<event::Gold>(
//			[this](const auto& event) { _gold = std::to_string(event.value); });
//	}
//
//	void onExit()
//	{
//		_listener.unlistenAll();
//	}
//
// private:
//	std::string _gold = "0";
//	Listener _listener;
//};
//
//// Shop button is only enabled when we have some gold (odd decision but for sample good :P)
// class ShopButton
//{
// public:
//	ShopButton(const std::shared_ptr<EventBus>& eventBus)
//		: _listener{eventBus}
//	{
//		// We can use lambda or bind your choice
//		_listener.listen<event::Gold>(
//			std::bind(&ShopButton::onGoldUpdated, this, std::placeholders::_1));
//		// Also we use RAII idiom to handle unlisten
//	}
//
//	bool isEnabled() const
//	{
//		return _isEnabled;
//	}
//
// private:
//	Listener _listener;
//	bool _isEnabled = false;
//
//	void onGoldUpdated(const event::Gold& event)
//	{
//		_isEnabled = event.value > 0;
//		std::cout << "Shop button is:" << _isEnabled << std::endl; // some kind of logs
//	}
//};

struct AA {
    int v;
};

struct BB {
    int v;
};

int main(int argc, char* argv[]) {
    auto eventBus = std::make_shared<EventBus>();
    Listener listener{eventBus};
    auto id_a1 = listener.listen<AA>([](const AA& aa) { std::cout << "aa1: " << aa.v << std::endl; });
    auto id_a2 = listener.listen<AA>([](const AA& aa) { std::cout << "aa2: " << aa.v << std::endl; });
    auto id_b1 = listener.listen<BB>([](const BB& aa) { std::cout << "bb: " << aa.v << std::endl; });
    std::cout << "=============" << std::endl;
    std::cout << "ids: " << *id_a1 << std::endl;
    std::cout << "ids: " << *id_a2 << std::endl;
    std::cout << "ids: " << *id_b1 << std::endl;

    eventBus->postpone(AA{100});
    eventBus->postpone(AA{123});
    eventBus->postpone(BB{8888});
    eventBus->postpone(AA{4534634});
    eventBus->postpone(AA{99878});
    eventBus->postpone(BB{212222222});
    eventBus->postpone(BB{999999999});
    eventBus->processLimit(3);
    listener.unlisten<AA>(0);
    std::cout << "=============" << std::endl;
    eventBus->process();
    eventBus->process();
    // Character characterController{eventBus};

    // UIWallet wallet{eventBus}; // UIWallet doesn't know anything about character
    //						   // or even who store gold
    // ShopButton shopButton{eventBus}; // ShopButton doesn't know anything about character
    //{
    //	wallet.onEnter();
    //}

    // wallet.onDraw();
    //{
    //	characterController.kill(Monster::Tux);
    //	eventBus->process();
    //}
    // wallet.onDraw();

    //// It is easy to test UI eg.
    // eventBus->postpone(event::Gold{1});
    // eventBus->process();
    // assert(shopButton.isEnabled() == true);

    // eventBus->postpone(event::Gold{0});
    // eventBus->process();
    // assert(shopButton.isEnabled() == false);

    // wallet.onExit();

    return 0;
}
