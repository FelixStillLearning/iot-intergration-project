#include "observable.h"

void Observable::add_observer(std::shared_ptr<Observer> observer) {
    if (!observer) {
        spdlog::warn("Observable: Attempted to add null observer");
        return;
    }

    std::lock_guard<std::mutex> lock(observer_mutex_);
    observers_.push_back(observer);
    spdlog::info("Observable: Observer registered - {}", observer->observer_name());
}

void Observable::remove_observer(std::shared_ptr<Observer> observer) {
    std::lock_guard<std::mutex> lock(observer_mutex_);

    // std::remove_if memindahkan elemen yang match ke akhir,
    // lalu erase menghapusnya (Erase-Remove idiom)
    auto it = std::remove_if(observers_.begin(), observers_.end(),
        [&observer](const std::shared_ptr<Observer>& o) {
            return o == observer;
        });

    if (it != observers_.end()) {
        spdlog::info("Observable: Observer removed - {}", observer->observer_name());
        observers_.erase(it, observers_.end());
    }
}

void Observable::notify_observers(const iot::SensorRequest& request) {
    std::lock_guard<std::mutex> lock(observer_mutex_);

    spdlog::debug("Observable: Notifying {} observer(s)", observers_.size());

    for (auto& observer : observers_) {
        if (observer) {
            observer->on_sensor_data(request);
        }
    }
}
