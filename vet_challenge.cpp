#include "Veteran.h"

void Veteran::challenge()
{
    while (true)
    {
        m_challenge.lock();
        unsigned who;
        bool answer;
        if (!challenge_ask(who, answer))
            continue;

        if (answer == false)
        {
            m_challenge.unlock();
            continue;
        }

        if (!challenge_cant_I_duel(who))
            continue;

        int data[2] = { t_id, static_cast<int>(who) };
        send_vet_readiness(t_id, who);

        send_new_state_notification("Preparing to a duel, telling everybody I (" + std::to_string(t_id) + ") and " + std::to_string(who) + " are busy");

        unsigned second[2];
        if (!challenge_find_seconds(second))
            continue;

        send_new_state_notification("Pojedynek pomiedzy " + std::to_string(t_id) + " i " + std::to_string(who) + " trwa!");

        if (!challenge_duel(who))
            continue;

        send_sec_readiness(second[0]);
        send_sec_readiness(second[1]);
    }
}

bool Veteran::challenge_ask(unsigned &who, bool &answer)
{
    send_new_state_notification("Trying to challenge someone");
    {
        const std::lock_guard<std::mutex> guard(m_busy);
        //m_chalenge will be freed by results thread
        
        if (!find_free_vet(who))
        {
            //m_chalenge will be freed by veteran readiness thread
            send_new_state_notification("Awaiting free veterans");
            b_waiting_vet = true;
            return false;
        }
        MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::vet_challenge, MPI_COMM_WORLD);
        send_new_state_notification("Challenging " + std::to_string(who) + " and waiting for their answer");
    }

    char ret;
    MPI_Recv(&ret, 1, MPI_CHAR, who, Message::vet_answer_challenge, MPI_COMM_WORLD, nullptr);
    answer = static_cast<bool>(ret);
    send_new_state_notification(std::string("Got a ") + (answer ? "positive" : "negative") + " answer from " + std::to_string(who));
    return true;
}

bool Veteran::challenge_cant_I_duel(unsigned who)
{
    const std::lock_guard<std::mutex> guard(m_busy);
    //m_chalenge will be freed by results thread
    if (b_busy)
    {
        int data[] = { t_id, true };
        MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::vet_duel_results, MPI_COMM_WORLD);
        send_new_state_notification("I need to cancel my duel with " + std::to_string(who));
        return false;
    }
    b_busy = true;
    return true;
}

bool Veteran::challenge_find_seconds(unsigned second[2])
{
    while (true)
    {
        m_waiting_free_sec.lock();

        send_new_state_notification("Trying to find a second");
        
        failed_answer:
        {
            const std::lock_guard<std::mutex> guard(m_waiting_sec);
            //todo: lepsze PRNG
            if (find_free_sec(second[0]))
            {
                //m_chalenge will be freed by veteran readiness thread
                send_new_state_notification("Awaiting free seconds");
                b_waiting_sec = true;
                continue;
            }
            MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, second[0], Message::vet_sec_ask, MPI_COMM_WORLD);
            send_new_state_notification("Asking " + std::to_string(second[0]) + " and waiting for their answer");
        }

        int answer[2];
        MPI_Recv(&answer, 2, MPI_INT, second[0], Message::vet_answer_challenge, MPI_COMM_WORLD, nullptr);
        if (answer == false)
        {
            m_waiting_free_sec.unlock();
            goto failed_answer;
        }
        second[1] = answer[2];
    }
    m_waiting_free_sec.unlock();
    return true;
}

bool Veteran::challenge_duel(int opponent)
{
    //todo: platform independent sleep
    _sleep(200 + rand() % 1000);
    char result = rand() % 2;
    MPI_Send(&result, 1, MPI_CHAR, 0, Message::vet_duel_results, MPI_COMM_WORLD);
    result ^= 1;
    MPI_Send(&result, 1, MPI_CHAR, opponent, Message::vet_duel_results, MPI_COMM_WORLD);

    return true;
}