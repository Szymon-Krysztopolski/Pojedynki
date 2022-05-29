#include "Second.h"

bool Second::findFreeSecond(unsigned& random_free, unsigned ID)
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

void Second::confirm()
{
    newStateNotification("Confirm thread started");

    MPI_Status status;
    while (true)
    {
        int vet;
        MPI_Recv(&vet, 1, MPI_INT, MPI_ANY_SOURCE, Message::secConfirm, MPI_COMM_WORLD, &status);
        int id = status.MPI_SOURCE;
        newStateNotification("Received a confirmation " + std::to_string(id) + " can take part in a duel of " + std::to_string(vet) + " as a second Second");
        {
            std::lock_guard<std::mutex> lock(busy_m);
            if (bBusy)
            {
                newStateNotification("Unfortunately for them I am no longer interested in participation in the old duel with " + std::to_string(vet) + ", because I have a new one. Sending to everyone that the Second " + std::to_string(id) + " is actually free");
                sendSecondReadiness_toVeterans(id);
                int data[2] = { false, id };
                MPI_Send(data, 2, MPI_INT, vet, Message::vetSecond_answer, MPI_COMM_WORLD);
                continue;
            }
            else
                setBusy(id);
            int data[2] = { true, id };
            sendSecondReadiness_toAll(tID, id);
            sendSecondReadiness_toAll(id, tID);
            MPI_Send(data, 2, MPI_INT, vet, Message::vetSecond_answer, MPI_COMM_WORLD);
            newStateNotification("Accepting duel of " + std::to_string(vet));
        }
    }
}

void Second::readiness_second()
{
    newStateNotification("Second readiness thread started");

    readiness(tID, secondingOnes.get(), secondingOnesClock.get(), Message::secReady, W + S, &Second::newStateNotification,
        []
        {
            std::lock_guard<std::mutex> lock(busy_m);
            if ((bReserved || bBusy) && secondingOnes[tID - W] == -1)
            {
                newStateNotification("I am free");
                unsetBusy();
            }

            if (bWaiting_second)
            {
                unsigned freeOne;
                if (findFreeSecond(freeOne))
                {
                    int data[2] = { tID, reserved };
                    newStateNotification("A free second has shown up");
                    MPI_Send(data, 2, MPI_INT, W + freeOne, Message::secSecond_ask, MPI_COMM_WORLD);
                    bWaiting_second = false;
                    waitingSecond_cv.notify_one();
                }
            }
        }, W);
}

void Second::startWaiting()
{
    newStateNotification("Second waiting thread started");

    MPI_Status status;
    while (true)
    {
        int host;
        MPI_Recv(&host, 1, MPI_INT, MPI_ANY_SOURCE, Message::secNoFreeSecond, MPI_COMM_WORLD, &status);
        newStateNotification("There are no longer any free seconds. Waiting for one to show up");
        {
            std::unique_lock<std::mutex> lck(waitingSecond_m);
            waitingSecond_cv.wait(lck, [] { return !bWaiting_second; });
        }
        unsigned freeOne;
        if (findFreeSecond(freeOne))
        {
            int data[2] = {tID, host};
            newStateNotification("A free second is available: " + std::to_string(W + freeOne) + ". Asking them to join a duel");
            MPI_Send(data, 2, MPI_INT, W + freeOne, Message::secSecond_ask, MPI_COMM_WORLD);
        }
        else bWaiting_second = true;
    }
}
