#pragma once
#include "handlers/observer/observer.h"
#include <memory>

/**
 * IObservable Interface (Pure Virtual)
 * 
 * Kontrak untuk semua class yang bisa "diamati" (observed).
 * Class yang implement IObservable bisa:
 *   - add_observer()    → daftarkan observer baru
 *   - remove_observer() → hapus observer
 *   - notify_observers() → beritahu SEMUA observer yang terdaftar
 * 
 * Dalam project ini, SensorController implement IObservable.
 * Ketika data gRPC masuk, SensorController memanggil notify_observers()
 * dan semua handler yang terdaftar akan bereaksi.
 */
class IObservable {
public:
    virtual ~IObservable() = default;

    /**
     * Daftarkan observer baru.
     * Observer akan di-notify setiap kali ada data sensor masuk.
     */
    virtual void add_observer(std::shared_ptr<Observer> observer) = 0;

    /**
     * Hapus observer dari daftar.
     * Observer tidak akan menerima notifikasi lagi.
     */
    virtual void remove_observer(std::shared_ptr<Observer> observer) = 0;

    /**
     * Notify semua observer yang terdaftar tentang data sensor baru.
     * @param request Data sensor yang akan dikirim ke semua observers
     */
    virtual void notify_observers(const iot::SensorRequest& request) = 0;
};