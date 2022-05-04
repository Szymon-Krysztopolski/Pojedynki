#pragma once
#include "global.h"

class Second
{
public:
    static void init(int t_id, unsigned W, unsigned S)
    {
        Second::t_id = t_id;
        Second::W = W;
        Second::S = S;
        Second::seconding_ones = std::unique_ptr<int[]>(new int[S]);
        memset(seconding_ones.get(), -1, S);
    }

    //run all threads
    static void start()
    {
        //answer_vet_th       = std::make_unique<std::thread>(&Second::answer_vet);
        //answer_sec_th       = std::make_unique<std::thread>(&Second::answer_sec);
        //sec_readiness_th    = std::make_unique<std::thread>(&Second::sec_readiness);
        //confirm_th          = std::make_unique<std::thread>(&Second::confirm);
    }

    //join all threads
    static void join()
    {
        answer_vet_th       ->join();
        answer_sec_th       ->join();
        sec_readiness_th    ->join();
        confirm_th          ->join();

        answer_vet_th       = nullptr;
        answer_sec_th       = nullptr;
        sec_readiness_th    = nullptr;
        confirm_th          = nullptr;
    }

private:
    static void send_new_state_notification(std::string state)
    {
        //todo: it takes quite some time to save logs and its blocking the threads, we should invert some kind of buffering or a different method of logging (saving to a file?)
        std::lock_guard<std::mutex> lock(m_log);
        std::cout << "--------------------------" << std::endl;
        std::cout << "[ID = " << t_id << "; LC = " << lamport_clock << "] " << state << std::endl;

        std::cout << "Seconds status: ";
        for (unsigned i = 0u; i != S; ++i)
            std::cout << (seconding_ones[i] ? 'X' : 'O') << ' ';
        std::cout << std::endl;
    }

    //static void answer_vet();

    //static void answer_sec();

    //static void sec_readiness();

    //static void confirm();

    static unsigned W, S;

    //contains ids of the processes that other processes currently interact with
    static std::unique_ptr<int[]>   seconding_ones;

    static bool        b_busy, b_reserved, b_waiting_sec;

    static int         t_id;

    static unsigned    lamport_clock;

    static std::unique_ptr<std::thread> answer_vet_th, answer_sec_th, sec_readiness_th, confirm_th;
    static std::mutex m_busy, m_waiting_sec, m_log;
};