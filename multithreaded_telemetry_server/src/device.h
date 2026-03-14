#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct DeviceState {
    std::string device_id_;
    double temperature_ = 0.0;
    double humidity_ = 0.0;
    double pressure_ = 0.0;
    std::chrono::system_clock::time_point last_update_;

    //
    bool IsStale() const {
        auto now = std::chrono::system_clock::now();
        return (now - last_update_) > std::chrono::minutes(5);
    }
};

class DeviceRegistry {
public:
    void UpdateDevice(const DeviceState &state) {
        std::unique_lock lock(mutex_);
        devices_[state.device_id_] = state;
    }

    std::optional<DeviceState> GetDevice(const std::string &id) const {
        std::shared_lock lock(mutex_);
        auto it = devices_.find(id);
        if (it != devices_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::vector<DeviceState> GerAllDevices() const {
        std::shared_lock lock(mutex_);
        std::vector<DeviceState> result;
        result.reserve(devices_.size());
        for (const auto& [_, state] : devices_) {
            result.push_back(state);
        }
        return result;
    }

private:
    std::unordered_map<std::string, DeviceState> devices_;
    mutable std::shared_mutex mutex_;
};