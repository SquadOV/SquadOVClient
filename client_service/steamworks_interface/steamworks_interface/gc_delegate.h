#pragma once

#include "shared/errors/error.h"
#include <memory>

namespace steamworks_interface {

class SteamworksGcDelegate {
public:
    virtual void handle(const void*, int size) = 0;
};
using SteamworksGcDelegatePtr = std::shared_ptr<SteamworksGcDelegate>;

template<typename T>
class TypedSteamworksGcDelegate: public SteamworksGcDelegate {
    using Delegate = std::function<void(T*)>;
public:
    TypedSteamworksGcDelegate(const Delegate& del):
        _del(del) 
    {}

    void handle(const void* data, int size) override {
        T typed;
        if (!typed.ParseFromArray(data, size)) {
            THROW_ERROR("Failed to parse Steamworks GC message.");
        }
        _del(&typed);
    }

private:
    Delegate _del;
};

class RawSteamworksGcDelegate: public SteamworksGcDelegate {
    using Delegate = std::function<void(const void*, int size)>;
public: 
    RawSteamworksGcDelegate(const Delegate& del):
        _del(del) 
    {}

    void handle(const void* data, int size) override {
        _del(data, size);
    }
private:
    Delegate _del;
};

}