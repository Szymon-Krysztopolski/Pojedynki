#include "Veteran.h"

void Veteran::challenge()
{
    newStateNotification("Challenge thread started");
    while (true)
    {
        {
            std::unique_lock<std::mutex> lck(busy_m);
            challenge_cv.wait(lck, [] { return !bBusy && !bWaiting_veteran; });
        }
        unsigned who;
        bool answer;
        if (!ask_challenge(who, answer))
            continue;

        if (answer == false)
            continue;

        if (!isCancelled_challenge(who))
            continue;

        //newStateNotification("Preparing to a duel, telling everybody I (" + std::to_string(tID) + ") and " + std::to_string(who) + " are busy");

        #ifndef NO_SECONDS_DEBUG
        unsigned second[2];
        if (!findSeconds_challenge(second))
            continue;
        #endif

        newStateNotification("A duel between " + std::to_string(tID) + " and " + std::to_string(who) 
#ifndef NO_SECONDS_DEBUG 
            + " with Seconds " + std::to_string(second[0] + W) + " and " + std::to_string(second[1] + W)
#endif
            + " has started!");

        if (!duel_challenge(who))
            continue;

        #ifndef NO_SECONDS_DEBUG
        sendSecondReadiness_toAll(second[0]);
        sendSecondReadiness_toAll(second[1]);
        #endif
    }
}

bool Veteran::ask_challenge(unsigned &who, bool &answer)
{
    newStateNotification("Trying to challenge someone");
    {
        if (!findFreeVeteran(who))
        {
            //m_chalenge will be freed by veteran readiness thread
            newStateNotification("Awaiting free veterans");
            bWaiting_veteran = true;
            return false;
        }
        MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::vetChallenge_ask, MPI_COMM_WORLD);
        newStateNotification("Challenging " + std::to_string(who) + " and waiting for their answer");
    }
    char ret;
    MPI_Status status;
    int clock = fightingOnesClock[who];
    MPI_Recv(&ret, 1, MPI_CHAR, who, Message::vetChallenge_answer, MPI_COMM_WORLD, &status);
    answer = static_cast<bool>(ret);
    {
        std::lock_guard<std::mutex> lock(busy_m);
        if (!answer && clock == fightingOnesClock[who])
            fightingOnes[who] = tID;
    }
    newStateNotification(std::string("Got a ") + (answer ? "positive" : "negative") + " answer from " + std::to_string(who));
    return answer;
}

bool Veteran::isCancelled_challenge(unsigned who)
{
    std::lock_guard<std::mutex> guard(busy_m);
    if (bBusy)
    {
        char data = 2;
        MPI_Send(&data, 1, MPI_CHAR, who, Message::vetResults, MPI_COMM_WORLD);
        newStateNotification("I need to cancel my duel with " + std::to_string(who));
        return false;
    }
    setBusy(who);
    sendVeteranReadiness_toVeterans(tID, who);
    return true;
}

bool Veteran::findSeconds_challenge(unsigned second[2])
{
    MPI_Status status;
    int answer[2];
    newStateNotification("Trying to find the Seconds");

    while (true)
    {
        {
            std::unique_lock<std::mutex> lck(waiting_second_m);
            second_cv.wait(lck, [] { return !bWaiting_second; });
        }

        if (!findFreeSecond(second[0]))
        {
            newStateNotification("Awaiting free Seconds");
            bWaiting_second = true;
            continue;
        }
        MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, W + second[0], Message::vetSecond_ask, MPI_COMM_WORLD);
        newStateNotification("Asking second " + std::to_string(W + second[0]) + " and waiting for their answer");

        MPI_Recv(answer, 2, MPI_INT, MPI_ANY_SOURCE, Message::vetSecond_answer, MPI_COMM_WORLD, &status);
        if (answer[0] == false)
        {
            std::unique_lock<std::mutex> lck(busy_m);
            secondingOnes[second[0]] = W + 1;
            continue;
        }

        second[1] = answer[1] - W;
        return true;
    }
    return true;
}

bool Veteran::duel_challenge(int opponent)
{
    //todo: platform independent sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(30000 + rand() % 300));
    char result = rand() % 2;
    MPI_Send(&result, 1, MPI_CHAR, tID, Message::vetResults, MPI_COMM_WORLD);
    result ^= 1;
    MPI_Send(&result, 1, MPI_CHAR, opponent, Message::vetResults, MPI_COMM_WORLD);
    newStateNotification("A duel between " + std::to_string(tID) + " and " + std::to_string(opponent) + " was won by " + (result ? std::to_string(opponent) : std::to_string(tID)));

    return true;
}