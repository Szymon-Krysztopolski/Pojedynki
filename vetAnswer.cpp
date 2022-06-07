#include "Veteran.h"

void Veteran::answerTheCallOfDuty()
{
    newStateNotification("Answer thread started");
    MPI_Status status;
    while (true)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, local_Comm, &status);
        switch (status.MPI_TAG)
        {
            case vetChallenge_ask:
            {
                std::lock_guard<std::mutex> guard(busy_m);
                answer();
            }
        break;
            case vetResults:
                result();
                //Notify the challenge thread we are no longer busy and it should check if we can challenge others again
                veteran_cv.notify_one();
        break;
            default:
                newStateNotification("Answer thread: unknown tag");
        break;
        }
    }
}

void Veteran::answer()
{
    MPI_Status status;
    recv_vetChallenge_ask(&status);
    int id = status.MPI_SOURCE;
    newStateNotification("We have been challenged to a duel from " + std::to_string(id));
    //std::lock_guard<std::mutex> guard(busy_m);
    //Simple yes/no based on [bBusy]
    if (bBusy)
    {
        char answer = false;
        send_vetChallenge_answer(id, answer);
        newStateNotification("Declining a duel from " + std::to_string(id));
    }
    else 
    {
        setBusy(id);
        char answer = true;
        send_vetChallenge_answer(id, answer);
        newStateNotification("Accepting a duel from " + std::to_string(id));
    }
}

void Veteran::result()
{
    char data;
    MPI_Status status;
    recv_vetResults(&status, &data);
    //Cancelled duel
    if (data == 2)
    {
        newStateNotification("A duel with " + std::to_string(data) + " has been cancelled");
        {
            std::lock_guard<std::mutex> lock(busy_m);
            unsetBusy();
        }
    }
    //Proper duel ending is done in this function
    else endDuel_result(data);
}

void Veteran::endDuel_result(bool won)
{
    if (!won)
    {
        //4fun message, saying someone is wounded. No on asked though. Good for debugging because now we know who is bleeding out without room in the hospital
        sendVeteranReadiness_toVeterans(tID, -2);
        while (true)
        {
#ifdef DEBUG
            newStateNotification("W IN");
#endif
            {
                std::unique_lock<std::mutex> lck2(wounded_m);
                //Waiting for a free bed
                wounded_cv.wait(lck2, [] {
                    bool bFreeBed = false;
                    for (unsigned i = 0; i != SZ; ++i)
                        if (hospitalBeds[i] == -1)
                        {
                            bFreeBed = true;
                            break;
                        }
                    return bFreeBed;
                    });
            }
#ifdef DEBUG
            newStateNotification("W OUT");
#endif
            //If we were fast enough to snatch the bed from other bleeding out Veterans, we can heal
            if (criticalSection(true)) break;
        }
        newStateNotification("Tending to my wounds");
        std::this_thread::sleep_for(std::chrono::milliseconds(HEAL_BASE_TIME + rand() % (1 + HEAL_RAND_TIME)));
        newStateNotification("I am healed!");
        //Free the bed
        criticalSection(false);
    }
    {
        std::lock_guard<std::mutex> lock(busy_m);
        unsetBusy();

        sendVeteranReadiness_toVeterans(tID);
    }
}