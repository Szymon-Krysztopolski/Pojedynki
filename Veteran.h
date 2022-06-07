#pragma once
#include "global.h"

class Veteran
{
public:
    static void init(int tID, unsigned W, unsigned S, unsigned SZ)
    {
        Veteran::tID        = tID;
        Veteran::W          = W;
        Veteran::S          = S;
        Veteran::SZ         = SZ;

        //Allocate all the tables
        fightingOnes        = std::unique_ptr<int[]>(new int[W]);
        fightingOnesClock   = std::unique_ptr<int[]>(new int[W]);
        secondingOnes       = std::unique_ptr<int[]>(new int[S]);
        secondingOnesClock  = std::unique_ptr<int[]>(new int[S]);
        lamport             = std::unique_ptr<int[]>(new int[W]);
        lamportClock        = std::unique_ptr<int[]>(new int[W]);
        hospitalBeds        = std::unique_ptr<int[]>(new int[SZ]);
        hospitalBedsClock   = std::unique_ptr<int[]>(new int[SZ]);
        hospitalDoor        = std::unique_ptr<bool[]>(new bool[W]);
        hospitalDoorClock   = std::unique_ptr<int[]>(new int[W]);

        //And fill them with defaults
        memset(fightingOnes.get(),          -1, W * sizeof(int));
        memset(fightingOnesClock.get(),     0,  W * sizeof(int));
        memset(secondingOnes.get(),         -1, S * sizeof(int));
        memset(secondingOnesClock.get(),    0,  S * sizeof(int));
        memset(lamport.get(),               0,  W * sizeof(int));
        memset(lamportClock.get(),          0,  W * sizeof(int));
        memset(hospitalBeds.get(),          -1, SZ * sizeof(int));
        memset(hospitalBedsClock.get(),     0,  SZ * sizeof(int));
        memset(hospitalDoor.get(),          0,  W * sizeof(bool));
        memset(hospitalDoorClock.get(),     0,  W * sizeof(int));

        newStateNotification(std::to_string(tID) + " initialized");
    }

    //run all threads
    static void start()
    {
        //Creating a thread runs it immediately
        answer_th               = std::make_unique<std::thread>(&Veteran::answerTheCallOfDuty);
        readiness_th            = std::make_unique<std::thread>(&Veteran::readyForEverything);
        critical_th             = std::make_unique<std::thread>(&Veteran::criticalThinking);

        //Barrier makes sure that the challenge thread will not send messages before creation of receiving threads (which would result in a crash because of MPI specification)
        MPI_Barrier(MPI_COMM_WORLD);
        challenge_th            = std::make_unique<std::thread>(&Veteran::challenge);
    }

    //join all threads
    //they will never join though
    //infinite loops
    static void join()
    {
        answer_th               ->join();
        readiness_th            ->join();
        critical_th             ->join();

        challenge_th            ->join();
    }

private:
    //Thread challenging other Veterans
    static void challenge();
        //Subroutines for the challenging thread (they return "true" whether the execution flow should be altered):
        //Asks another Veteran for a duel
        static bool ask_challenge(unsigned *who);
        //Asks a Second to find second Second and have them seconding the duel
        static void findSeconds_challenge(unsigned (*second)[2]);
        //Check whether the duel was cancelled by the host
        static bool isCancelled_challenge(unsigned who);
        //The ongoing duel (threads sleeping)
        static void duel_challenge(int opponent);

    //Thread answering the Veterans and Seconds
    static void answerTheCallOfDuty();
        //Subroutine dedicated to answering challenges of other veterans
        static void answer();

    //Procedure awaiting results of the duel
    static void result();
        static void endDuel_result(bool won);

    //Critical seciton
    static bool criticalSection(bool bEnter);
        static void criticalSection_enter();
        static bool criticalSection_leave(int hospitalBed, bool bEnter);
    //Thread listening to leaving the critical section by other processes
    static void criticalThinking();
        static void criticalSectionLeft();
        //Door opening and closing of Lamport's bakery algorithm
        static void hospital_open();
        static void hospital_close();


    //Veteran readiness (fightingOnes) and second readiness (secondingOnes) thread
    static void readyForEverything();
        //Vet
        static void sendVeteranReadiness_toVeterans(int who, int with_who = -1);
        static bool findFreeVeteran(unsigned *random_free);
        //Second
        static void sendSecondReadiness_toAll(int who, int with_who = -1);
        static bool findFreeSecond(unsigned *random_free);

    static void setBusy(int who)
    {
        //bBusy is added only for clarity reasons. We could have only checked fightingOnes[tID] instead of bBusy
        bBusy = true;
        fightingOnes[tID] = who;
    };
    
    static void unsetBusy()
    {
        bBusy = false;
        fightingOnes[tID] = -1;
    };

    //Prints a message to the standard output with some additional info about fightingOnes, secondingOnes and hospitalBeds
    static void newStateNotification(std::string state)
    {
        std::string message = "--------------------------\n[VET][ID = " + std::to_string(tID) + "; LC = " + std::to_string(myLamport) + "] " + state;

        message += "\nWorthy opponents status: ";
        for (unsigned i = 0u; i != W; ++i)
        {
            if (fightingOnes[i] == -2)
            {
                bool bWounded = false;
                for (unsigned j = 0u; j != SZ; ++j)
                    if (hospitalBeds[j] == i)
                    {
                        bWounded = true;
                        break;
                    }
                message += (bWounded ? "W " : "D ");
            }
            else if (fightingOnes[i] == -1)
                message += "O ";
            else
                message += "X ";
        }
        
        message += "\nSeconds status: ";
        for (unsigned i = 0u; i != S; ++i)
            message += ((secondingOnes[i] == -1)    ? "O " : "X ");

        message += "\nHospital beds status: ";
        for (unsigned i = 0u; i != SZ; ++i)
            message += ((hospitalBeds[i] == -1)     ? "O " : "X ");
        
        std::lock_guard<std::mutex> lock(log_m);
        std::cout << message << std::endl << std::flush;
    }

    //The parameters passed via commandline
    static unsigned                     W, S, SZ;
    
    //The lamport clock of entire critical section. Should always be equal to te sum of hospitalBedsClock array (which is an array of Lamport clocks of internal incremental changes of the resources inside the critical section)
    static unsigned myLamport;

    //Array mapping every Veteran to another Veteran they are dueling with or -1 if they are free. Clocks are used to prevent an invalid state if sleeps are disabled/set very low and messages from other processes happen very fast, 
    //possibly sending invalid (a new one is valid and received, but another process still didn't send the first -now invalid- one and it will override it) state as the order is not guaranteed
    //There is a chance one Veteran will change its state from busy to free then again to busy very quickly by finishing one duel and then accepting a duel of someone else
    //MPI doesn't guarantee order of messages sent by different processes, so we could receive busy->free messages instead of free->busy
    static std::unique_ptr<int[]>       fightingOnes, fightingOnesClock;

    //Alike to fightingOnes array, but maps a Second tID to a Veteran tID. It indicates that the Second is seconding in a duel hosted by this Veteran
    static std::unique_ptr<int[]>       secondingOnes, secondingOnesClock;

    //Hospital beds, a limited resource governed by a critical section, where Veterans rest after receiving injuries
    static std::unique_ptr<int[]>       hospitalBeds, hospitalBedsClock;

    //Tickets in Lamport's bakery algorithm
    static std::unique_ptr<int[]>       lamport, lamportClock;

    //Entering array or so called "doors" of Lamport's algorithm    
    static std::unique_ptr<bool[]>      hospitalDoor;
    static std::unique_ptr<int[]>       hospitalDoorClock;

    //A more elegant way of saying "fightingOnes[tID]"
    static bool                         bBusy;
    
    //Current process ID
    static int                          tID;

    //All of the threads
    static std::unique_ptr<std::thread> challenge_th,       answer_th,          readiness_th,       critical_th;
    //All of the mutexes protecting variables or intertwined with conditional waiting
    static std::mutex                   challenge_m,        busy_m,             wounded_m,          log_m,                      second_m,           lamport_m,          door_m;
    //All of the condition variables
    static std::condition_variable      veteran_cv,         second_cv,          lamport_cv,         wounded_cv,                 door_cv;

};