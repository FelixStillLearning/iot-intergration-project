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
    void add_observer(std::shared_ptr<Observer> observer) override;

    /**
     * Hapus observer dari list.
     * Setelah dihapus, observer tidak akan menerima notifikasi lagi.
     */
    void remove_observer(std::shared_ptr<Observer> observer) override;

    /**
     * Notify SEMUA observer yang terdaftar.
     * Memanggil on_sensor_data() pada setiap observer secara berurutan.
     * 
     * @param request Data sensor dari gRPC yang akan diberitahukan
     */
    void notify_observers(const iot::SensorRequest& request) override;

protected:
    /// Daftar observer yang terdaftar
    std::vector<std::shared_ptr<Observer>> observers_;

    /// Mutex untuk thread-safety (gRPC concurrent requests)
    std::mutex observer_mutex_;
};
