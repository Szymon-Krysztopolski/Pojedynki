#include "Second.h"

bool Second::findFreeSecond(unsigned& random_free)
{
    return findFree(secondingOnes.get(), S, random_free);
}

void Second::sendSecondReadiness_toVeterans(int who, int with_who)
{
    sendReadiness(tID, secondingOnes.get(), Message::vetReady, W, who, with_who);
}

void Second::sendSecondReadiness_toAll(int who, int with_who)
{
    sendReadiness(tID, secondingOnes.get(), Message::secReady, W + S, who, with_who);
}

void Second::confirm()
{
    MPI_Status status;
    int id[2];
    while (true)
    {
        MPI_Recv(id, 2, MPI_INT, MPI_ANY_SOURCE, Message::secConfirm, MPI_COMM_WORLD, &status);
        newStateNotification("Recieved a confirmation " + std::to_string(id[0]) + " can take part in a duel as a second Second");
        {
            const std::lock_guard<std::mutex> lock(busy_m);
            if (bBusy)
            {
                newStateNotification("Unfortunately for them I am no longer interested in participation in the old duel, because I have a new one. Sending to everyone that the confirming second is actually free");
                sendSecondReadiness_toAll(id[0]);
                continue;
            }
            else
                bBusy = true;
            int data[2] = { true, id[0] };
            if (bReserved)
            {
                //todo: lepsze PRNG
                bool bWhose = rand() % 2;
                data[0] = bWhose;
                MPI_Send(data, 2, MPI_INT, reserved, Message::vetSecond_answer, MPI_COMM_WORLD);
                data[0] = !bWhose;
                MPI_Send(data, 2, MPI_INT, id[0], Message::vetSecond_answer, MPI_COMM_WORLD);
                newStateNotification("Accepting duel of " + (bWhose ? std::to_string(tID) : std::to_string(id[0])));
            }
            else
            {
                MPI_Send(data, 2, MPI_INT, reserved, Message::vetSecond_answer, MPI_COMM_WORLD);
                newStateNotification("Accepting duel of " + std::to_string(tID));
            }
        }
    }
}

void Second::readiness_second()
{
    readiness(tID, secondingOnes.get(), Message::secReady, &Second::newStateNotification,
        []
        {
            if (secondingOnes[tID] == -1)
            {
                const std::lock_guard<std::mutex> lock(busy_m);
                bBusy = false;
                bReserved = false;
                newStateNotification("I am no longer busy");
            }
            const std::lock_guard<std::mutex> lock(waitingSecond_m);
            if (bWaiting_second)
            {
                unsigned freeOne;
                if (findFreeSecond(freeOne))
                {
                    MPI_Send(&tID, 1, MPI_INT, freeOne, Message::vetSecond_ask, MPI_COMM_WORLD);
                    newStateNotification("A free second has shown up");
                    bWaiting_second = false;
                }
            }
        });
}

void Second::startWaiting()
{
    MPI_Status status;
    while (true)
    {
        MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::secNoFreeSecond, MPI_COMM_WORLD, &status);
        newStateNotification("There are no longer any free seconds. Waiting for one to show up");
        const std::lock_guard<std::mutex> lock(waitingSecond_m);
        unsigned freeOne;
        if (findFreeSecond(freeOne))
        {
            MPI_Send(&freeOne, 1, MPI_INT, freeOne, Message::secSecond_ask, MPI_COMM_WORLD);
            newStateNotification("A free second is available. Asking them to join a duel");
        }
        else bWaiting_second = true;
    }
}
