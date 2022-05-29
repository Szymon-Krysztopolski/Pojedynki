#include "Second.h"

void Second::invitedByVeteran()
{
    newStateNotification("Veteran invitation considering thread started");

    MPI_Status status;
    while (true)
    {
        MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vetSecond_ask, MPI_COMM_WORLD, &status);
        newStateNotification(std::to_string(status.MPI_SOURCE) + " asked me to participate in a duel hosted by them");
        
        {
            std::lock_guard<std::mutex> lock(busy_m);
            if (bBusy || bReserved)
            {
                newStateNotification("I am busy. Declining invitation from " + std::to_string(status.MPI_SOURCE));
                int data[2] = { false, tID };
                MPI_Send(data, 2, MPI_INT, status.MPI_SOURCE, Message::vetSecond_answer, MPI_COMM_WORLD);
                continue;
            }
            else
            {
                newStateNotification("Considering invitation of " + std::to_string(status.MPI_SOURCE) + " to their duel and trying to find a second Second");
                bReserved = true;
                reserved = status.MPI_SOURCE;
                sendSecondReadiness_toVeterans(tID, reserved);
            }
            unsigned freeOne;
            if (findFreeSecond(freeOne))
            {
                int data[2] = { tID, status.MPI_SOURCE };
                newStateNotification("A free second is available: " + std::to_string(W + freeOne) + ". Asking them to join a duel hosted by " + std::to_string(status.MPI_SOURCE));
                MPI_Send(data, 2, MPI_INT, W + freeOne, Message::secSecond_ask, MPI_COMM_WORLD);
            }
            else bWaiting_second = true;
        }
    }
}

void Second::answerSecond()
{
    newStateNotification("Second invitation considering thread started");

    MPI_Status status;
    while (true)
    {
        int data[2];
        MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, Message::secSecond_ask, MPI_COMM_WORLD, &status);
        newStateNotification(std::to_string(status.MPI_SOURCE) + " asked me to participate in a duel hosted by " + std::to_string(data[1]));
        {
            std::lock_guard<std::mutex> lock(busy_m);
            if (bBusy)
            {
                unsigned notUsed;
                if (findFreeSecond(notUsed, data[0]))
                {
                    newStateNotification("I am busy. Maybe " + std::to_string(W + notUsed) + " can take part in this duel hosted by " + std::to_string(data[1]) + " instead");
                    MPI_Send(data, 2, MPI_INT, W + notUsed, Message::secSecond_ask, MPI_COMM_WORLD);
                }
                else
                {
                    newStateNotification("I am busy. And there is no one that can take part in this duel hosted by " + std::to_string(data[1]) + ". Informing the Second, who asked");
                    MPI_Send(data + 1, 1, MPI_INT, data[0], Message::secNoFreeSecond, MPI_COMM_WORLD);
                }
            }
            else
            {
                setBusy(data[1]);
                newStateNotification("I accept to be a second Second in a duel hosted by " + std::to_string(data[1]));
                MPI_Send(data + 1, 1, MPI_INT, data[0], Message::secConfirm, MPI_COMM_WORLD);
            }
        }
    }
}

