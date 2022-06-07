#include "Second.h"

void Second::readyForNothing()
{
    newStateNotification("Readiness thread started");
    MPI_Status status;
    while (true)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, readiness_Comm, &status);
        switch (status.MPI_TAG)
        {
            case secReady:
            {
                std::lock_guard<std::mutex> guard(secbusy_m);
                readiness(tID, secondingOnes.get(), secondingOnesClock.get(), Message::secReady, W + S, &Second::newStateNotification, W);
                if ((bReserved || bBusy) && secondingOnes[tID - W] == -1)
                {
                    newStateNotification("I am free");
                    unsetBusy();
                }
            }
                waitingSecond_cv.notify_one();
        break;
        default:
            newStateNotification("Readiness thread: unknown tag");
            break;
        }
    }
}

void Second::waiting()
{
    newStateNotification("Waiting thread started");
    MPI_Status status;
    while (true)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, waiting_Comm, &status);
        switch (status.MPI_TAG)
        {
            case secNoFreeSecond:
                waitingSecond();
        break;
            default:
                newStateNotification("Waiting thread: unknown tag");
        break;
        }
    }
}


bool Second::findFreeSecond(unsigned *random_free, unsigned ID)
{
    return findFree(secondingOnes.get(), S, random_free, false, ID - W);
}

void Second::sendSecondReadiness_toVeterans(int who, int with_who)
{
    sendReadiness(tID, secondingOnes.get(), secondingOnesClock.get(), Message::secReady, W, who - W, with_who, W);
}

void Second::sendSecondReadiness_toAll(int who, int with_who)
{
    sendReadiness(tID, secondingOnes.get(), secondingOnesClock.get(),  Message::secReady, W + S, who - W, with_who, W);
}

void Second::waitingSecond()
{
    MPI_Status status;
    recv_secNoFreeSecond(&status);
    newStateNotification("There are no longer any free Seconds. Waiting for one to show up");

    //Waiting for a free Second to show up
    unsigned freeOne;
    std::unique_lock<std::mutex> lck(secbusy_m);
    waitingSecond_cv.wait(lck, [&freeOne]
        { 
            return findFreeSecond(&freeOne); 
        });
    //Maybe somebody asked us to participate in a duel and we no longer need to search for a second Second
    if (bBusy)
    {
        newStateNotification("I found a duel while waiting for a free Second");
        return;
    }
    newStateNotification("A free Second is available: " + std::to_string(W + freeOne) + ". Asking them to join a duel hosted by " + std::to_string(reserved));
    //int data[2] = { tID, host };
    int data[2] = { tID, reserved };
    send_secSecond_ask(W + freeOne, data);
    secondingOnes[freeOne] = tID;
}
