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
    sendReadiness(tID, fightingOnes.get(), Message::vetReady, W, who, with_who);
}

void Veteran::sendSecondReadiness_toAll(int who, int with_who)
{
    sendReadiness(tID, secondingOnes.get(), Message::secReady, W + S, who, with_who);
}

void Veteran::readiness_veterans()
{
    readiness(tID, fightingOnes.get(), Message::vetReady, &Veteran::newStateNotification,
        [] 
        {
            if (fightingOnes[tID] == -1)
            {
                const std::lock_guard<std::mutex> lock(busy_m);
                bBusy = false;
            }
            if (bWaiting_veteran)
            {
                unsigned notUsed;
                if (findFreeVeteran(notUsed))
                {
                    newStateNotification("A free veteran has shown up");
                    bWaiting_veteran = false;
                    challenge_m.unlock();
                }
            }
        });    
}

void Veteran::readiness_seconds()
{
    readiness(tID, secondingOnes.get(), Message::secReady, &Veteran::newStateNotification,
        []
        {
            const std::lock_guard<std::mutex> lock(waiting_second_m);
            if (bWaiting_second)
            {
                unsigned notUsed;
                if (findFreeSecond(notUsed))
                {
                    newStateNotification("A free second has shown up");
                    bWaiting_second = false;
                    waiting_untilFreeSecond_m.unlock();
                }
            }
        });
}