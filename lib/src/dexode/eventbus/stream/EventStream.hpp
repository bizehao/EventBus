#pragma once

#include <any>
#include <cstdint>
#include <stdexcept>
#include <memory>

namespace dexode::eventbus::stream {

class EventStream {
public:
    virtual ~EventStream() = default;

    virtual void postpone(std::any event) = 0;
    virtual std::size_t process(std::size_t limit = std::numeric_limits<std::size_t>::max()) = 0;

    virtual std::shared_ptr<uint32_t> addListener(std::any callback) = 0;
    virtual bool removeListener(std::uint32_t listenerID) = 0;

    virtual uint32_t listener_count() = 0;
};

class NoopEventStream : public EventStream {
public:
    void postpone(std::any event) override { throw std::runtime_error{"Noop"}; }
    size_t process(std::size_t limit) override { throw std::runtime_error{"Noop"}; }
    std::shared_ptr<uint32_t> addListener(std::any callback) override { throw std::runtime_error{"Noop"}; }
    bool removeListener(std::uint32_t listenerID) override { throw std::runtime_error{"Noop"}; }
    uint32_t listener_count() override { throw std::runtime_error{"Noop"}; }
};

} // namespace dexode::eventbus::stream
