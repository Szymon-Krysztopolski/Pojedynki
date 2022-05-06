#pragma once
#include "global.h"

class Veteran
{
public:
    static void init(int tID, unsigned W, unsigned S, unsigned SZ)
    {
        Veteran::tID = tID;
        Veteran::W = W;
        Veteran::S = S;
        Veteran::SZ = SZ;
        Veteran::fightingOnes  = std::unique_ptr<int[]>(new int[W]);
        Veteran::secondingOnes = std::unique_ptr<int[]>(new int[S]);
        memset(fightingOnes.get(),     -2, W);
        memset(secondingOnes.get(),    -1, S);

        //at the beginning all veterans are considered not ready, so we are waiting for one to be available
        bWaiting_veteran = true;
        challenge_m.lock();
    }

    //run all threads
    static void start()
    {
        challenge_th            = std::make_unique<std::thread>(&Veteran::challenge);
        answer_th               = std::make_unique<std::thread>(&Veteran::answer);
        result_th               = std::make_unique<std::thread>(&Veteran::result);
        readiness_veterans_th   = std::make_unique<std::thread>(&Veteran::readiness_veterans);
        readiness_seconds_th    = std::make_unique<std::thread>(&Veteran::readiness_seconds);
        freeBed_th              = std::make_unique<std::thread>(&Veteran::freeBed);
    }

    //join all threads
    static void join()
    {
        challenge_th            ->join();
        answer_th               ->join();
        result_th               ->join();
        readiness_veterans_th   ->join();
        readiness_seconds_th    ->join();
        freeBed_th              ->join();

        challenge_th            = nullptr;
        answer_th               = nullptr;
        result_th               = nullptr;
        readiness_veterans_th   = nullptr;
        readiness_seconds_th    = nullptr;
        freeBed_th              = nullptr;
    }

private:
    //first thread, challenging other veterans
    static void challenge();
    //subroutines (returning boolean wheter execution flow should be altered):
    static bool ask_challenge(unsigned& who, bool& answer);
    static bool findSeconds_challenge(unsigned second[2]);
    static bool isCancelled_challenge(unsigned who);
    static bool duel_challenge(int opponent);

    //second thread, answering challenges of other veterans
    static void answer();

    //third thread, awaiting results of the duel
    static void result();
    static void endDuel_result(bool won);

    //critical seciton
    static bool criticalSection();
    static void enter_criticalSection();
    static void leave_criticalSection();

    //fourth thread, awaiting messages of readiness of other veterans
    static void readiness_veterans();
    static void sendVeteranReadiness_toVeterans(int who, int with_who = -1);
    static bool findFreeVeteran(unsigned& random_free);

    //fifth thread, awaiting messages of readiness of seconds
    static void readiness_seconds();
    static void sendSecondReadiness_toAll(int who, int with_who = -1);
    static bool findFreeSecond(unsigned& random_free);

    //sixth thread, awaiting messages of freed beds
    static void freeBed();

    static void newStateNotification(std::string state)
    {
        //todo: it takes quite some time to save logs and its blocking the threads, we should invert some kind of buffering or a different method of logging (saving to a file?)
        std::lock_guard<std::mutex> lock(log_m);
        std::cout << "--------------------------" << std::endl;
        std::cout << "[ID = " << tID << "; LC = " << lamportClock << "] " << state << std::endl;

        std::cout << "Worthy opponents status: ";
        for (unsigned i = 0u; i != W; ++i)
            std::cout << (fightingOnes[i] ? 'X' : 'O') << ' ';
        std::cout << std::endl;

        std::cout << "Seconds status: ";
        for (unsigned i = 0u; i != S; ++i)
            std::cout << (secondingOnes[i] ? 'X' : 'O') << ' ';
        std::cout << std::endl;
    }

    static unsigned W, S, SZ;

    //contains ids of the processes that other processes currently interact with
    static std::unique_ptr<int[]>   fightingOnes,
                                    secondingOnes;

    static bool        bBusy, bWaiting_veteran, bWaiting_second, bWounded;

    static int         tID;

    static unsigned    lamportClock;

    static std::unique_ptr<std::thread> challenge_th, answer_th, result_th, readiness_veterans_th, readiness_seconds_th, freeBed_th;
    static std::mutex challenge_m, busy_m, waiting_untilFreeSecond_m, waiting_second_m, wounded_m, log_m;
};