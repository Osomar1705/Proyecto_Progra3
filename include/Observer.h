#ifndef OBSERVER_H
#define OBSERVER_H

#include <algorithm>
#include <vector>

// ---------------------------------------------------------
// PATRON DE DISENO: OBSERVER
// ---------------------------------------------------------
// Subject/Observer completo: LikeSubject mantiene la lista de observadores y
// dispara notify() cuando cambia el conjunto de likes. Los observadores
// implementan ILikeObserver (p. ej. el motor de recomendaciones).

class ILikeObserver {
public:
    virtual void onLikedMoviesChanged() = 0;
    virtual ~ILikeObserver() = default;
};

class LikeSubject {
public:
    void attach(ILikeObserver* observer) {
        observers_.push_back(observer);
    }

    void detach(ILikeObserver* observer) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                         observers_.end());
    }

    void notifyLikedMoviesChanged() {
        for (ILikeObserver* observer : observers_) {
            observer->onLikedMoviesChanged();
        }
    }

private:
    std::vector<ILikeObserver*> observers_; // sin propiedad: los observadores viven mas que el subject
};

#endif // OBSERVER_H
