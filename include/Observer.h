#ifndef OBSERVER_H
#define OBSERVER_H

// ---------------------------------------------------------
// PATRON DE DISENO: OBSERVER
// ---------------------------------------------------------
class ILikeObserver {
public:
    virtual void onLikedMoviesChanged() = 0;
    virtual ~ILikeObserver() = default;
};

#endif // OBSERVER_H
