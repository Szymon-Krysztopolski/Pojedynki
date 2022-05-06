#include "Veteran.h"
#define NO_SECONDS_DEBUG

void Veteran::challenge()
{
    while (true)
    {
        challenge_m.lock();
        unsigned who;
        bool answer;
        if (!ask_challenge(who, answer))
            continue;

        if (answer == false)
        {
            challenge_m.unlock();
            continue;
        }

        if (!(who))
            continue;

        int data[2] = { tID, static_cast<int>(who) };
        sendVeteranReadiness_toVeterans(tID, who);

        newStateNotification("Preparing to a duel, telling everybody I (" + std::to_string(tID) + ") and " + std::to_string(who) + " are busy");

        #ifndef NO_SECONDS_DEBUG
        unsigned second[2];
        if (!findSeconds_challenge(second))
            continue;
        #endif

        newStateNotification("A duel between " + std::to_string(tID) + " and " + std::to_string(who) + " has started!");

        if (!duel_challenge(who))
            continue;

        #ifndef NO_SECONDS_DEBUG
        sendSecondReadiness_all(second[0]);
        sendSecondReadiness_all(second[1]);
        #endif
    }
}

bool Veteran::ask_challenge(unsigned &who, bool &answer)
{
    newStateNotification("Trying to challenge someone");
    {
        const std::lock_guard<std::mutex> guard(busy_m);
        //m_chalenge will be freed by results thread
        
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
    MPI_Recv(&ret, 1, MPI_CHAR, who, Message::vetChallenge_answer, MPI_COMM_WORLD, &status);
    answer = static_cast<bool>(ret);
    newStateNotification(std::string("Got a ") + (answer ? "positive" : "negative") + " answer from " + std::to_string(who));
    return true;
}

bool Veteran::isCancelled_challenge(unsigned who)
{
    const std::lock_guard<std::mutex> guard(busy_m);
    //m_chalenge will be freed by results thread
    if (bBusy)
    {
        int data[] = { tID, true };
        MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::vetResults, MPI_COMM_WORLD);
        newStateNotification("I need to cancel my duel with " + std::to_string(who));
        return false;
    }
    bBusy = true;
    return true;
}

bool Veteran::findSeconds_challenge(unsigned second[2])
{
    MPI_Status status;
    int answer[2];
    while (true)
    {
        waiting_untilFreeSecond_m.lock();

        newStateNotification("Trying to find the Seconds");
        
        failedAnswer:
        {
            const std::lock_guard<std::mutex> guard(waiting_second_m);
            //todo: lepsze PRNG
            if (findFreeSecond(second[0]))
            {
                //m_chalenge will be freed by veteran readiness thread
                newStateNotification("Awaiting free Seconds");
                bWaiting_second = true;
                continue;
            }
            MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, second[0], Message::vetSecond_ask, MPI_COMM_WORLD);
            newStateNotification("Asking Second " + std::to_string(second[0]) + " and waiting for their answer");
        }

        MPI_Recv(&answer, 2, MPI_INT, second[0], Message::vetSecond_answer, MPI_COMM_WORLD, &status);
        if (answer[0] == false)
            goto failedAnswer;  //actually a good use of goto
        
        second[1] = answer[1];
    }
    waiting_untilFreeSecond_m.unlock();
    return true;
}

bool Veteran::duel_challenge(int opponent)
{
    //todo: platform independent sleep
    _sleep(200 + rand() % 1000);
    char result = rand() % 2;
    MPI_Send(&result, 1, MPI_CHAR, 0, Message::vetResults, MPI_COMM_WORLD);
    result ^= 1;
    MPI_Send(&result, 1, MPI_CHAR, opponent, Message::vetResults, MPI_COMM_WORLD);
    newStateNotification("A duel between " + std::to_string(tID) + " and " + std::to_string(opponent) + " has ended");

    return true;
}