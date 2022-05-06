#pragma once
#include "global.h"

class Second
{
public:
    static void init(int t_id, unsigned W, unsigned S)
    {
        Second::tID = tID;
        Second::W = W;
        Second::S = S;
        Second::secondingOnes = std::unique_ptr<int[]>(new int[S]);
        memset(secondingOnes.get(), -1, S);
    }

    //run all threads
    static void start()
    {
        answerSecond_th     = std::make_unique<std::thread>(&Second::answerSecond);
        answerVeteran_th    = std::make_unique<std::thread>(&Second::invitedByVeteran);
        readiness_second_th = std::make_unique<std::thread>(&Second::readiness_second);
        confirm_th          = std::make_unique<std::thread>(&Second::confirm);
        startWaiting_th     = std::make_unique<std::thread>(&Second::confirm);
    }

    //join all threads
    static void join()
    {
        answerSecond_th     ->join();
        answerVeteran_th    ->join();
        readiness_second_th ->join();
        confirm_th          ->join();
        startWaiting_th     ->join();

        answerSecond_th     = nullptr;
        answerVeteran_th    = nullptr;
        readiness_second_th = nullptr;
        confirm_th          = nullptr;
        startWaiting_th     = nullptr;
    }

private:
    static void newStateNotification(std::string state)
    {
        //todo: it takes quite some time to save logs and its blocking the threads, we should invert some kind of buffering or a different method of logging (saving to a file?)
        std::lock_guard<std::mutex> lock(log_m);
        std::cout << "--------------------------" << std::endl;
        std::cout << "[ID = " << tID << "; LC = " << lamportClock << "] " << state << std::endl;

        std::cout << "Seconds status: ";
        for (unsigned i = 0u; i != S; ++i)
            std::cout << (secondingOnes[i] ? 'X' : 'O') << ' ';
        std::cout << std::endl;
    }

    //first thread
    static void answerSecond();

    //second thread
    static void invitedByVeteran();

    //third thread
    static void readiness_second();
    static void sendSecondReadiness_toVeterans(int who, int with_who = -1);
    static void sendSecondReadiness_toAll(int who, int with_who = -1);
    static bool findFreeSecond(unsigned& random_free);

    //fourth thread
    static void startWaiting();
    
    //fifth thread
    static void confirm();

    static unsigned W, S;

    //contains ids of the processes that other processes currently interact with
    static std::unique_ptr<int[]>   secondingOnes;

    static bool        bBusy, bReserved, bWaiting_second;

    static int         tID, reserved;

    static unsigned    lamportClock;

    static std::unique_ptr<std::thread> answerSecond_th, answerVeteran_th, readiness_second_th, confirm_th, startWaiting_th;
    static std::mutex busy_m, waitingSecond_m, log_m;
};