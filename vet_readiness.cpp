#include "Veteran.h"

bool Veteran::find_free_vet(unsigned& random_free)
{
    return find_free(fighting_ones.get(), W, random_free);
}

bool Veteran::find_free_sec(unsigned& random_free)
{
    return find_free(seconding_ones.get(), S, random_free);
}

void Veteran::send_vet_readiness(int who, int with_who)
{
    send_readiness(t_id, fighting_ones.get(), Message::vet_ready, W, who, with_who);
}

void Veteran::send_sec_readiness(int who, int with_who)
{
    send_readiness(t_id, seconding_ones.get(), Message::sec_ready, W + S, who, with_who);
}

void Veteran::vet_readiness()
{
    readiness(t_id, fighting_ones.get(), Message::vet_ready, &Veteran::send_new_state_notification,
        [] 
        {
            if (fighting_ones[t_id] == -1)
            {
                const std::lock_guard<std::mutex> lock(m_busy);
                b_busy = false;
            }
            if (b_waiting_vet)
            {
                unsigned notUsed;
                if (find_free_vet(notUsed))
                {
                    b_waiting_vet = false;
                    m_challenge.unlock();
                }
            }
        });    
}

void Veteran::sec_readiness()
{
    readiness(t_id, seconding_ones.get(), Message::sec_ready, &Veteran::send_new_state_notification,
        []
        {
            const std::lock_guard<std::mutex> lock(m_waiting_sec);
            if (b_waiting_sec)
            {
                unsigned notUsed;
                if (find_free_sec(notUsed))
                {
                    b_waiting_sec = false;
                    m_waiting_free_sec.unlock();
                }
            }
        });
}