#include "Veteran.h"

void Veteran::readyForEverything()
{
    newStateNotification("Readiness thread started");
    MPI_Status status;
    while (true)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, readiness_Comm, &status);
        switch (status.MPI_TAG)
        {
            case vetReady:
            {
                std::lock_guard<std::mutex> guard(busy_m);
                readiness(tID, fightingOnes.get(), fightingOnesClock.get(), Message::vetReady, W, &Veteran::newStateNotification);
            }
            veteran_cv.notify_one();
        break;
            case secReady:
            {
                std::lock_guard<std::mutex> guard(second_m);
                readiness(tID, secondingOnes.get(), secondingOnesClock.get(), Message::secReady, W + S, &Veteran::newStateNotification, W);
            }
            second_cv.notify_one();
        break;
            default:
                newStateNotification("Readiness thread: unknown tag");
        break;
        }
    }
}

bool Veteran::findFreeVeteran(unsigned *random_free)
{
    return findFree(fightingOnes.get(), W, random_free, true, tID);
}

bool Veteran::findFreeSecond(unsigned *random_free)
{
    return findFree(secondingOnes.get(), S, random_free);
}

void Veteran::sendVeteranReadiness_toVeterans(int who, int with_who)
{
    sendReadiness(tID, fightingOnes.get(), fightingOnesClock.get(),     Message::vetReady, W, who, with_who);
    //There is always symmetry. If one is busy with someone, the other is too. -1 is a mark being free, -2 of being wounded
    if (with_who != -1 && with_who != -2)
        sendReadiness(tID, fightingOnes.get(), fightingOnesClock.get(), Message::vetReady, W, with_who, who);
}

void Veteran::sendSecondReadiness_toAll(int who, int with_who)
{
    sendReadiness(tID, secondingOnes.get(), secondingOnesClock.get(),   Message::secReady, W + S, who, with_who, W);
}
