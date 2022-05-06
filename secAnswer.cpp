#include "Second.h"

void Second::invitedByVeteran()
{
    MPI_Status status;
    int data[2];
    while (true)
    {
        MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vetSecond_ask, MPI_COMM_WORLD, &status);
        newStateNotification(std::to_string(status.MPI_SOURCE) + " asked me to participate in a duel hosted by them");
        {
            const std::lock_guard<std::mutex> lock(busy_m);
            if (bBusy || bReserved)
            {
                newStateNotification("I am busy. Declining invitation from " + std::to_string(status.MPI_SOURCE));
                data[0] = false;
                MPI_Send(data, 2, MPI_CHAR, status.MPI_SOURCE, Message::secSecond_answer, MPI_COMM_WORLD);
                continue;
            }
            else
            {
                bReserved = true;
                reserved = status.MPI_SOURCE;
                newStateNotification("Considering invitation of " + std::to_string(status.MPI_SOURCE) + " to their duel and trying to find a second Second");
                sendSecondReadiness_toVeterans(tID, reserved);
            }
        }
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

void Second::answerSecond()
{
    MPI_Status status;
    int data[2];
    while (true)
    {
        MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, Message::secSecond_ask, MPI_COMM_WORLD, &status);
        newStateNotification(std::to_string(data[0]) + " asked me to participate in a duel hosted by " + std::to_string(data[1]));
        const std::lock_guard<std::mutex> lock(busy_m);
        if (bBusy)
        {
            unsigned notUsed;
            if (findFreeSecond(notUsed))
            {
                newStateNotification("I am busy. Maybe " + std::to_string(notUsed) + " can take part in this duel hosted by " + std::to_string(data[1]) + " instead");
                MPI_Send(data, 2, MPI_INT, notUsed, Message::secSecond_answer, MPI_COMM_WORLD);
            }
            else
            {
                newStateNotification("I am busy. And there is no one that can take part in this duel hosted by " + std::to_string(data[1]) + ". Informing the Second, who asked");
                MPI_Send(data, 2, MPI_INT, data[0], Message::secSecond_answer, MPI_COMM_WORLD);
            }
        }
        else
        {
            bBusy = true;
            newStateNotification("I accept to be a second Second in a duel hosted by " + std::to_string(data[1]));
            MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, data[0], Message::secConfirm, MPI_COMM_WORLD);
        }
    }
}

