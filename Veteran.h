#pragma once
#include "global.h"

class Veteran
{
public:
    static void init(int t_id, unsigned W, unsigned S, unsigned SZ)
    {
        Veteran::t_id = t_id;
        Veteran::W = W;
        Veteran::S = S;
        Veteran::SZ = SZ;
        Veteran::fighting_ones  = std::unique_ptr<int[]>(new int[W]);
        Veteran::seconding_ones = std::unique_ptr<int[]>(new int[S]);
        memset(fighting_ones.get(),     -2, W);
        memset(seconding_ones.get(),    -1, S);

        //at the beginning all veterans are considered not ready, so we are waiting for one to be available
        b_waiting_vet = true;
        m_challenge.lock();
    }

    //run all threads
    static void start()
    {
        challenge_th        = std::make_unique<std::thread>(&Veteran::challenge);
        answer_th           = std::make_unique<std::thread>(&Veteran::answer);
        result_th           = std::make_unique<std::thread>(&Veteran::result);
        vet_readiness_th    = std::make_unique<std::thread>(&Veteran::vet_readiness);
        sec_readiness_th    = std::make_unique<std::thread>(&Veteran::sec_readiness);
        free_bed_th         = std::make_unique<std::thread>(&Veteran::free_bed);
    }

    //join all threads
    static void join()
    {
        challenge_th    ->join();
        answer_th       ->join();
        result_th       ->join();
        vet_readiness_th->join();
        sec_readiness_th->join();
        free_bed_th     ->join();

        challenge_th    = nullptr;
        answer_th       = nullptr;
        result_th       = nullptr;
        vet_readiness_th= nullptr;
        sec_readiness_th= nullptr;
        free_bed_th     = nullptr;
    }

private:
    //first thread, challenging other veterans
    static void challenge();
    //subroutines (returning boolean wheter execution flow should be altered):
    static bool challenge_ask(unsigned& who, bool& answer);
    static bool challenge_find_seconds(unsigned second[2]);
    static bool challenge_cant_I_duel(unsigned who);
    static bool challenge_duel(int opponent);

    //second thread, answering challenges of other veterans
    static void answer();

    //third thread, awaiting results of the duel
    static void result();
    static void result_end_duel(bool won);
    static bool result_critical_section(bool won);

    //fourth thread, awaiting messages of readiness of other veterans
    static void vet_readiness();
    static bool find_free_vet(unsigned& random_free);
    static void send_vet_readiness(int who, int with_who = -1);

    //fifth thread, awaiting messages of readiness of seconds
    static void sec_readiness();
    static bool find_free_sec(unsigned& random_free);
    static void send_sec_readiness(int who, int with_who = -1);

    //sixth thread, awaiting messages of freed beds
    static void free_bed();

    static void send_new_state_notification(std::string state)
    {
        //todo: it takes quite some time to save logs and its blocking the threads, we should invert some kind of buffering or a different method of logging (saving to a file?)
        std::lock_guard<std::mutex> lock(m_log);
        std::cout << "--------------------------" << std::endl;
        std::cout << "[ID = " << t_id << "; LC = " << lamport_clock << "] " << state << std::endl;

        std::cout << "Worthy opponents status: ";
        for (unsigned i = 0u; i != W; ++i)
            std::cout << (fighting_ones[i] ? 'X' : 'O') << ' ';
        std::cout << std::endl;

        std::cout << "Seconds status: ";
        for (unsigned i = 0u; i != S; ++i)
            std::cout << (seconding_ones[i] ? 'X' : 'O') << ' ';
        std::cout << std::endl;
    }

    /* todo: później zobaczyć, czy będzie dało się w prosty sposób uprościć kod w tym stylu
    bool get_busy()
    {
        const std::lock_guard<std::mutex> guard(m_busy);
        return b_busy;
    }

    void set_busy(bool value)
    {
        const std::lock_guard<std::mutex> guard(m_busy);
        b_busy = value;
    }

    bool set_if_not_set_busy()
    {
        const std::lock_guard<std::mutex> guard(m_busy);
        if (b_busy)
            return false;
        b_busy = true;
        return true;
    }

    bool get_waiting_vet()
    {
        const std::lock_guard<std::mutex> guard(m_waiting_vet);
        return b_waiting_vet;
    }

    void set_waiting_vet(bool value)
    {
        const std::lock_guard<std::mutex> guard(m_waiting_vet);
        b_waiting_vet = value;
    }

    bool set_if_not_set_waiting_vet()
    {
        const std::lock_guard<std::mutex> guard(m_waiting_vet);
        if (b_waiting_vet)
            return false;
        b_waiting_vet = true;
        return true;
    }

    //returns t_id of a random free veteran
    unsigned set_if_no_free_ones_waiting_vet()
    {
        const std::lock_guard<std::mutex> guard(m_waiting_vet);
        std::vector<int> freeOnes;
        freeOnes.reserve(W);

        for (unsigned i = 0; i != W; ++i)
            if (fighting_ones[i] == -1)
                freeOnes.push_back(i);

        //todo: lepsze PRNG
        if (freeOnes.empty())
        {
            b_waiting_vet = true;
            return -1;
        }
        return rand() % freeOnes.size();
    }

    bool get_waiting_sec()
    {
        const std::lock_guard<std::mutex> guard(m_waiting_sec);
        return b_waiting_sec;
    }

    void set_waiting_sec(bool value)
    {
        const std::lock_guard<std::mutex> guard(m_waiting_sec);
        b_waiting_sec = value;
    } 
    
    bool set_if_not_set_waiting_sec()
    {
        const std::lock_guard<std::mutex> guard(m_waiting_sec);
        if (b_waiting_sec)
            return false;
        b_waiting_sec = true;
        return true;
    }

    //returns t_id of a random free second
    unsigned set_if_no_free_ones_waiting_sec()
    {
        const std::lock_guard<std::mutex> guard(m_waiting_sec);
        std::vector<int> freeOnes;
        freeOnes.reserve(S);

        for (unsigned i = 0; i != S; ++i)
            if (seconding_ones[i] == -1)
                freeOnes.push_back(i);

        //todo: lepsze PRNG
        if (freeOnes.empty())
        {
            b_waiting_sec = true;
            return -1;
        }
        return rand() % freeOnes.size();
    }
    */
    static unsigned W, S, SZ;

        //contains ids of the processes that other processes currently interact with
    static std::unique_ptr<int[]>   fighting_ones,
                                    seconding_ones;

    static bool        b_busy, b_waiting_vet, b_waiting_sec, b_wounded;

    static int         t_id;

    static unsigned    lamport_clock;

    static std::unique_ptr<std::thread> challenge_th, answer_th, result_th, vet_readiness_th, sec_readiness_th, free_bed_th;
    static std::mutex m_challenge, m_busy, m_waiting_free_sec, m_waiting_sec, m_wounded, m_log;
};