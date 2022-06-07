#include "Second.h"

void Second::talkToTheOverlord()
{
    newStateNotification("Veteran communication thread started");
    MPI_Status status;
    while (true)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        switch (status.MPI_TAG)
        {
            case vetSecond_ask:
            {
                std::lock_guard<std::mutex> lck(secbusy_m);
                invitedByVeteran();
            }
            waitingSecond_cv.notify_one();
        break;
                default:
            newStateNotification("Veteran communication thread: unknown tag");
        break;
        }
    }
}

void Second::answerForTheCrimes()
{
    newStateNotification("Second answering thread started");
    MPI_Status status;
    while (true)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, local_Comm, &status);
        switch (status.MPI_TAG)
        {
            case secSecond_ask:
            {
                std::lock_guard<std::mutex> lock(secbusy_m);
                answerSecond();
            }
                waitingSecond_cv.notify_one();
        break;
            case secConfirm:
            {
                std::lock_guard<std::mutex> lock(secbusy_m);
                confirm();
            }
            waitingSecond_cv.notify_one();
        break;
            default:
                newStateNotification("Second answering thread: unknown tag");
        break;
        }
    }
}

void Second::invitedByVeteran()
{
    MPI_Status status;
    recv_vetSecond_ask(&status);
    newStateNotification(std::to_string(status.MPI_SOURCE) + " asked me to participate in a duel hosted by them");
    //decline
    if (bBusy || bReserved)
    {
        newStateNotification("I am busy. Declining invitation from " + std::to_string(status.MPI_SOURCE));
        int data[2] = { false, tID };
        send_vetSecond_answer(status.MPI_SOURCE, data);
        return;
    }
    //try to accept
    else
    {
        newStateNotification("Considering invitation of " + std::to_string(status.MPI_SOURCE) + " to their duel and trying to find a second Second");
        bReserved = true; //to not accept invites from Veterans again
        reserved = status.MPI_SOURCE; //store to use later for confirmation message
        sendSecondReadiness_toVeterans(tID, reserved);
    }

#ifdef DEBUG
    newStateNotification("IsFree");
#endif
    unsigned freeOne;
    if (findFreeSecond(&freeOne))
    {
        //accept
        newStateNotification("A free Second is available: " + std::to_string(W + freeOne) + ". Asking them to join a duel hosted by " + std::to_string(status.MPI_SOURCE));
        int data[2] = { tID, status.MPI_SOURCE };
        send_secSecond_ask(W + freeOne, data);
        secondingOnes[freeOne] = status.MPI_SOURCE;
    }
    //Educate ourselves we need to wait for a free Second now
    else
    {
#ifdef DEBUG
        newStateNotification("NeverFree");
#endif

        send_secNoFreeSecond(tID);
    }
}

void Second::answerSecond()
{
    MPI_Status status;
    int data[2];
    recv_secSecond_ask(&status, &data);
    newStateNotification(std::to_string(status.MPI_SOURCE) + " asked me to participate in a duel hosted by " + std::to_string(data[1]));
    {
        //Decline if busy and try to ask someone else in asker's stead
        if (bBusy)
        {
            unsigned freeOne;
            if (findFreeSecond(&freeOne, data[0]))
            {
                newStateNotification("I am busy. Maybe " + std::to_string(W + freeOne) + " can take part in this duel hosted by " + std::to_string(data[1]) + " instead");
                send_secSecond_ask(W + freeOne, data);
                secondingOnes[freeOne] = tID;
            }
            else
            {
                newStateNotification("I am busy. And there is no one that can take part in this duel hosted by " + std::to_string(data[1]) + ". Informing the Second, who asked");
                send_secNoFreeSecond(data[0]);
            }
        }
        else
        {
            setBusy(data[1]);   
            newStateNotification("I accept to be a second Second in a duel hosted by " + std::to_string(data[1]));
            send_secConfirm(data[0]);
        }
    }
}

void Second::confirm()
{
    MPI_Status status;
    recv_secConfirm(&status);
    int id = status.MPI_SOURCE;
    newStateNotification("Received a confirmation " + std::to_string(id) + " can take part in a duel of " + std::to_string(reserved) + " as a second Second");
    //If we are busy it means we just have joined other duel and we need cancel ours
    //There is also a special case we need to take care of
    if (bBusy)
    {
        //There is a special case when we agree to be a second Second in the duel of a Second... who agreed to be a second Second in our duel
        //To fight this ambiguity, there is a special logic for this, solving the issue based on tID comparision
        if (id == agreed)
        {
            newStateNotification("I agreed to a duel invite by Second " + std::to_string(id) + " when they agreed to mine. One needs to be cancelled");
            int data[2] = { true, id };
            if (tID < id)
            {
                newStateNotification("Accepting duel of " + std::to_string(reserved) + " with second Second " + std::to_string(id));
                send_vetSecond_answer(reserved, data);
            }
            else
            {
                newStateNotification("Declining duel of " + std::to_string(reserved));
                data[0] = false;
                send_vetSecond_answer(reserved, data);
            }
            return;
        }
        //Normal case, we just decline
        newStateNotification("Unfortunately for them I am no longer interested in participation in the old duel with " + std::to_string(reserved) + ", because I have a new one. Sending to everyone that the Second " + std::to_string(id) + " is actually free");
        sendSecondReadiness_toAll(id);
        int data[2] = { false, id };
        send_vetSecond_answer(reserved, data);
    }
    //We are not busy, so we can agree to the duel and set ourselves and the second Second to be busy
    else
    {
        newStateNotification("Accepting duel of " + std::to_string(reserved) + " with second Second " + std::to_string(id));
        setBusy(id);

        int data[2] = { true, id };
        send_vetSecond_answer(reserved, data);

        sendSecondReadiness_toAll(tID, id);
        sendSecondReadiness_toAll(id, tID);
    }
}