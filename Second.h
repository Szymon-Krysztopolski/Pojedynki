#pragma once
#include "global.h"

class Second
{
public:
    static void init(int tID, unsigned W, unsigned S)
    {
        Second::tID = tID;
        Second::W = W;
        Second::S = S;
        
        //Allocate all the tables
        Second::secondingOnes       = std::unique_ptr<int[]>(new int[S]);
        Second::secondingOnesClock  = std::unique_ptr<int[]>(new int[S]);

        //And fill them with defaults
        memset(secondingOnes.get(),     -1, S * sizeof(int));
        memset(secondingOnesClock.get(), 0, S * sizeof(int));
        
        newStateNotification(std::to_string(tID) + " initialized");
    }

    //run all threads
    static void start()
    {
        veteran_th          = std::make_unique<std::thread>(&Second::talkToTheOverlord);
        readiness_th        = std::make_unique<std::thread>(&Second::readyForNothing);
        answer_th           = std::make_unique<std::thread>(&Second::answerForTheCrimes);
        waiting_th          = std::make_unique<std::thread>(&Second::confirm);
        //Need to call the barrier here to not lock the flow, but it has no use for Seconds, only Veterans
        MPI_Barrier(MPI_COMM_WORLD);
    }

    //join all threads
    //they will never join though
    //infinite loops
    static void join()
    {
        veteran_th  ->join();
        readiness_th->join();
        answer_th   ->join();
        waiting_th  ->join();
    }

private:
    //Prints a message to the standard output with some additional info about secondingOnes
    static void newStateNotification(std::string state)
    {
        std::string message = "--------------------------\n[SEC][ID = " + std::to_string(tID) + "] " + state + "\nSeconds status: ";
        for (unsigned i = 0u; i != S; ++i)
            message += ((secondingOnes[i] == -1) ? "O " : "X ");
        std::lock_guard<std::mutex> lock(log_m);
        std::cout << message << std::endl;
    }

    //Thread dedicated to answering Second invitations to duels in name of Veteran as a second Second
    static void answerForTheCrimes();
        static void answerSecond();
        //Seconds handshake or farewell
        //1. Second invited Second as a second Second, he agreed: handshake and a duel start
        //2. Second invited Second as a second Second, he agreed, but in meanwhile the Second who invited agreed to another duel, so this one needs to be cancelled: fare thy well!
        static void confirm();

    //All the message exchanges between this Second and Veterans
    static void talkToTheOverlord();
        //Answers invitation from the Veterans
        static void invitedByVeteran();

    //Second readiness (secondingOnes) thread
    static void readyForNothing();
        static void sendSecondReadiness_toVeterans(int who, int with_who = -1);
        static void sendSecondReadiness_toAll(int who, int with_who = -1);
        static bool findFreeSecond(unsigned *random_free, unsigned ID = tID);

    //A thread that works when no one else wants to! ...When there are no free Seconds to invite as second Seconds, so we ought to wait for one to appear
    //The method itself is waiting for a better name
    static void waiting();
        static void waitingSecond();


    static void setBusy(int who)
    {
        bBusy = true;
        secondingOnes[tID - W] = who;
        //If we are busy and still searching for a second Second, it means we participate in other duel and need not search for the second Second anymore
    };

    static void unsetBusy()
    {
        bBusy = false;
        bReserved = false;
        secondingOnes[tID - W] = -1;
    };

    //The parameters passed via commandline. Only those that we need
    static unsigned                     W, S;

    //Array mapping every Second to a Veteran they are seconding duel of or -1 if they are free. Clocks are used to prevent an invalid state if sleeps are disabled/set very low and messages from other processes happen very fast, 
    //possibly sending invalid (a new one is valid and received, but another process still didn't send the first -now invalid- one and it will override it) state as the order is not guaranteed
    //MPI doesn't guarantee order of messages sent by different processes, so we could receive busy->free messages instead of free->busy
    static std::unique_ptr<int[]>       secondingOnes,      secondingOnesClock;

    //A more elegant way of saying "secondingOnes[tID]"
    static bool                         bBusy;
    //A Second can be reserved by Veteran and considered busy by them, but not by the Seconds (we don't want to be asked by another Veteran to join their duel, but allow to be asked by Seconds to join their duels)
    //This is also why there are two version of sending readiness: toVeterans and toAll
    static bool                         bReserved;

    //Current process ID
    static int                          tID;
    //ID of Veterans we have been reserved by
    static int                          reserved;
    //ID of Second we agreed to, to be a second Second
    static int                          agreed;
    
    //All of the threads
    static std::unique_ptr<std::thread> veteran_th,         answer_th,          readiness_th,        waiting_th;
    //All of the mutexes protecting variables or intertwined with conditional waiting
    static std::mutex                   secbusy_m,          log_m;
    //All of the condition variables
    static std::condition_variable      waitingSecond_cv;
};