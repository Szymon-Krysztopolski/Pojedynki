#include "Veteran.h"

bool Veteran::findFreeVeteran(unsigned& random_free)
{
    return findFree(fightingOnes.get(), W, random_free, true, tID);
}

bool Veteran::findFreeSecond(unsigned& random_free)
{
    return findFree(secondingOnes.get(), S, random_free);
}

void Veteran::sendVeteranReadiness_toVeterans(int who, int with_who)
{
    sendReadiness(tID, fightingOnes.get(), fightingOnesClock.get(),   Message::vetReady, W, who, with_who);
    if (with_who != -1)
        sendReadiness(tID, fightingOnes.get(), fightingOnesClock.get(), Message::vetReady, W, with_who, who - W);
}

void Veteran::sendSecondReadiness_toAll(int who, int with_who)
{
    sendReadiness(tID, secondingOnes.get(), secondingOnesClock.get(), Message::secReady, W + S, who, with_who, W);
}

void Veteran::readiness_veterans()
{
    newStateNotification("Veterans readiness thread started");

    readiness(tID, fightingOnes.get(), fightingOnesClock.get(), Message::vetReady, W, &Veteran::newStateNotification,
        [] 
        {
            std::lock_guard<std::mutex> lock(busy_m);
            if (bBusy && fightingOnes[tID] == -1)
            {
                newStateNotification("I am free");
                unsetBusy();
            }
            if (bWaiting_veteran)
            {
                unsigned notUsed;
                if (findFreeVeteran(notUsed))
                {
                    newStateNotification("A free veteran has shown up");
                    bWaiting_veteran = false;
                    challenge_cv.notify_one();
                }
            }
        });    
}

void Veteran::readiness_seconds()
{
    newStateNotification("Seconds readiness thread started");

    readiness(tID, secondingOnes.get(), secondingOnesClock.get(), Message::secReady, W + S, &Veteran::newStateNotification,
        []
        {
            std::lock_guard<std::mutex> lck(waiting_second_m);
            if (bWaiting_second)
            {
                unsigned notUsed;
                if (findFreeSecond(notUsed))
                {
                    newStateNotification("A free second has shown up");
                    bWaiting_second = false;
                    second_cv.notify_one();
                }
            }
        }, W);
}