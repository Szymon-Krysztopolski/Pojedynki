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
        Second::secondingOnes       = std::unique_ptr<int[]>(new int[S]);
        Second::secondingOnesClock  = std::unique_ptr<int[]>(new int[S]);
        memset(secondingOnes.get(),     -1, S * sizeof(int));
        memset(secondingOnesClock.get(), 0, S * sizeof(int));
        newStateNotification("Initialized");
    }

    //run all threads
    static void start()
    {
        answerSecond_th     = std::make_unique<std::thread>(&Second::answerSecond);
        answerVeteran_th    = std::make_unique<std::thread>(&Second::invitedByVeteran);
        readiness_second_th = std::make_unique<std::thread>(&Second::readiness_second);
        confirm_th          = std::make_unique<std::thread>(&Second::confirm);
        startWaiting_th     = std::make_unique<std::thread>(&Second::startWaiting);
        MPI_Barrier(MPI_COMM_WORLD);
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
        //todo: it takes quite some time to save logs (and flush them using std::endl) and its blocking the threads, we should invert some kind of buffering or a different method of logging (saving to a file?)
        std::lock_guard<std::mutex> lock(log_m);
        std::string message = "--------------------------\n[SEC][ID = " + std::to_string(tID) + "; LC = " + std::to_string(lamportClock) + "] " + state + "\nSeconds status: ";
        for (unsigned i = 0u; i != S; ++i)
            message += ((secondingOnes[i] == -1) ? "O " : "X ");
        std::cout << message << std::endl;
    }

    //first thread
    static void answerSecond();

    //second thread
    static void invitedByVeteran();

    //third thread
    static void readiness_second();
    static void sendSecondReadiness_toVeterans(int who, int with_who = -1);
    static void sendSecondReadiness_toAll(int who, int with_who = -1);
    static bool findFreeSecond(unsigned& random_free, unsigned ID = tID);

    //fourth thread
    static void startWaiting();
    
    //fifth thread
    static void confirm();

    static void setBusy(int who)
    {
        bBusy = true;
        secondingOnes[tID - W] = who;
    };

    static void unsetBusy()
    {
        bBusy = false;
        bReserved = false;
        secondingOnes[tID - W] = -1;
    };

    static unsigned                     W, S;

    //contains ids of the processes that other processes currently interact with
    static std::unique_ptr<int[]>       secondingOnes,      secondingOnesClock;

    static bool                         bBusy,              bReserved,          bWaiting_second;

    static int                          tID,                reserved;

    static unsigned                     lamportClock;

    static std::unique_ptr<std::thread> answerSecond_th,    answerVeteran_th,   readiness_second_th,        confirm_th,         startWaiting_th;
    static std::mutex                   busy_m,             waitingSecond_m,    log_m;
    static std::condition_variable      waitingSecond_cv;
};