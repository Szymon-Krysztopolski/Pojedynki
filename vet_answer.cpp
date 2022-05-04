#include "Veteran.h"

void Veteran::answer()
{
	while (true)
	{
        MPI_Status status;
        MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vet_challenge, MPI_COMM_WORLD, &status);
        const std::lock_guard<std::mutex> guard(m_busy);
        if (b_busy)
        {
            char answer = false;
            MPI_Send(&answer, 1, MPI_CHAR, MPI_ANY_SOURCE, Message::vet_answer_challenge, MPI_COMM_WORLD);
        }
        else 
        {
            //todo: this is bad!! And in the graph we included this mistake too. If this is allowed then do this before asking for a duel otherwise we need some big changes
            if (t_id < status.MPI_SOURCE)
            {
                char answer = false;
                MPI_Send(&answer, 1, MPI_CHAR, MPI_ANY_SOURCE, Message::vet_answer_challenge, MPI_COMM_WORLD);
            }
            else
            {
                b_busy = true;
                char answer = true;
                MPI_Send(&answer, 1, MPI_CHAR, MPI_ANY_SOURCE, Message::vet_answer_challenge, MPI_COMM_WORLD);
            }
        }
	}
}