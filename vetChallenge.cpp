#include "Veteran.h"

void Veteran::challenge()
{
    newStateNotification("Challenge thread started");
    while (true)
    {
        //Check if a duel is in progress and wait for it to end
        unsigned who;
        {
#ifdef DEBUG
            newStateNotification("VET IN");
#endif
            std::unique_lock<std::mutex> lck(busy_m);
            veteran_cv.wait(lck, [&who] { return !bBusy && findFreeVeteran(&who); });
#ifdef DEBUG
            newStateNotification("VET OUT");
#endif
        }

        //Challenge someone to a duel
        if (!ask_challenge(&who))
            continue;

        //See if we accepted other duel and need to cancel ours
        if (!isCancelled_challenge(who))
            continue;

        //Find Seconds
        #ifndef NO_SECONDS_DEBUG
        unsigned second[2];
        findSeconds_challenge(&second);
        #endif

        newStateNotification("A duel between " + std::to_string(tID) + " and " + std::to_string(who) 
#ifndef NO_SECONDS_DEBUG 
            + " with Seconds " + std::to_string(second[0] + W) + " and " + std::to_string(second[1] + W)
#endif
            + " has started!");

        //Make the duel happen (we are the host and deciding its outcome)
        duel_challenge(who);

        //Free Seconds
        #ifndef NO_SECONDS_DEBUG
        {
            std::lock_guard<std::mutex> lock(second_m);
            sendSecondReadiness_toAll(second[0]);
            sendSecondReadiness_toAll(second[1]);
        }
        #endif
    }
}

bool Veteran::ask_challenge(unsigned *who)
{
    //Challege someone
    send_vetChallenge_ask(*who);
    newStateNotification("Challenging Veteran " + std::to_string(*who) + " and waiting for their answer");

    char ret;
    //Wait for the answer
    int clock = fightingOnesClock[*who];
    bool answer;
    MPI_Status status;
    recv_vetChallenge_answer(&status, &ret, *who);
    {
        std::lock_guard<std::mutex> lock(busy_m);
        answer = static_cast<bool>(ret);
        //If the clock didn't change during our waiting for answer, meaning [fightingOnes[*who]] didn't change either,
        //we can assume they are busy, but the message about it didn't arrive to us yet and set it on our own to not ask them again
        if (!answer && (clock == fightingOnesClock[*who]))
            fightingOnes[*who] = *who;
    }
    newStateNotification(std::string("Got a ") + (answer ? "positive" : "negative") + " answer from " + std::to_string(*who));
    return answer;
}

bool Veteran::isCancelled_challenge(unsigned who)
{
    std::lock_guard<std::mutex> guard(busy_m);
    //If we are busy it means we accepted other duel and need to cancel ours
    if (bBusy)
    {
        char data = 2;
        send_vetResults(who, data);
        newStateNotification("I need to cancel my duel with " + std::to_string(who));
        return false;
    }
    //Otherwise we state we say we are now busy
    setBusy(who);
    sendVeteranReadiness_toVeterans(tID, who);
    return true;
}

void Veteran::findSeconds_challenge(unsigned (*second)[2])
{
    MPI_Status status;
    int answer[2];
    newStateNotification("Trying to find the Seconds");

    while (true)
    {
        int clock;
        //If there are no Seconds available, wait for them
        {
            std::unique_lock<std::mutex> lck(second_m);
            second_cv.wait(lck, [&second] { return findFreeSecond(&((*second)[0])); });

            clock = secondingOnesClock[(*second)[0]];
        }
        newStateNotification("Asking Second " + std::to_string(W + (*second)[0]) + " and waiting for their answer");

        //Ask the presumably free Second
        send_vetSecond_ask(W + (*second)[0]);
        //Wait for the answer
        recv_vetSecond_answer(&status, &answer, W + (*second)[0]);

        if (answer[0] == false)
        {
            std::lock_guard<std::mutex> lck(second_m);
            //If the clock didn't change during our waiting for answer, meaning [fightingOnes[*who]] didn't change either,
            //we can assume they are busy, but the message about it didn't arrive to us yet and set it on our own to not ask them again
            if (clock == secondingOnesClock[(*second)[0]])
                secondingOnes[(*second)[0]] = W + (*second)[0];
            newStateNotification("Got a negative answer from " + std::to_string(W + (*second)[0]));
            continue;
        }
        //If the answer is "Yes!", we can find our other half ...of Seconds in the returned data
        (*second)[1] = answer[1] - W;
        newStateNotification("Got a positive answer from " + std::to_string(W + (*second)[0]));
        break;
    }
}

void Veteran::duel_challenge(int opponent)
{
    //We are fighting!
    std::this_thread::sleep_for(std::chrono::milliseconds(DUEL_BASE_TIME + rand() % (1 + DUEL_RAND_TIME)));
    //The fate holds the answer who won, as we all are glorious warriors of skill 
    char result = rand() % 2;
    //Talking to myself about the results, reflecting and searching for a way to improve
    send_vetResults(tID, result);
    result ^= 1;
    //And talking to the sane one - person I was dueling with
    send_vetResults(opponent, result);
    newStateNotification("A duel between " + std::to_string(tID) + " and " + std::to_string(opponent) + " was won by " + (result ? std::to_string(opponent) : std::to_string(tID)));
}