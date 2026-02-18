#pragma once
#include "adapters/abstract_adapters/observable/interface_observable.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <mutex>
#include <spdlog/spdlog.h>

/**
 * Observable (Concrete Base Class)
 * 
 * Implementasi konkret dari IObservable.
 * Menyimpan daftar observers dan menyediakan method notify_observers()
 * yang memanggil on_sensor_data() pada setiap observer.
 * 
 * Class ini menggunakan mutex untuk thread-safety karena:
 *   - gRPC bisa handle multiple request secara concurrent
 *   - Observer bisa ditambah/dihapus dari thread lain
 * 
 * Inheritance chain:
 *   IObservable (interface)
 *       └── Observable (concrete base) ← INI
 *               └── SensorController (final class)
 */
class Observable : public IObservable {
public:
    /**
     * Daftarkan observer baru ke list.
     * Observer akan di-notify setiap kali notify_observers() dipanggil.
     */
    void add_observer(std::shared_ptr<Observer> observer) override {
        if (!observer) {
            spdlog::warn("Observable: Attempted to add null observer");
            return;
        }

        std::lock_guard<std::mutex> lock(observer_mutex_);
        observers_.push_back(observer);
        spdlog::info("Observable: Observer registered - {}", observer->observer_name());
    }

    /**
     * Hapus observer dari list.
     * Setelah dihapus, observer tidak akan menerima notifikasi lagi.
     */
    void remove_observer(std::shared_ptr<Observer> observer) override {
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

    /**
     * Notify SEMUA observer yang terdaftar.
     * Memanggil on_sensor_data() pada setiap observer secara berurutan.
     * 
     * @param request Data sensor dari gRPC yang akan diberitahukan
     */
    void notify_observers(const iot::SensorRequest& request) override {
        std::lock_guard<std::mutex> lock(observer_mutex_);

        spdlog::debug("Observable: Notifying {} observer(s)", observers_.size());

        for (auto& observer : observers_) {
            if (observer) {
                observer->on_sensor_data(request);
            }
        }
    }

protected:
    /// Daftar observer yang terdaftar
    std::vector<std::shared_ptr<Observer>> observers_;

    /// Mutex untuk thread-safety (gRPC concurrent requests)
    std::mutex observer_mutex_;
};
