#include "Veteran.h"

void Veteran::answer()
{
	while (true)
	{
        MPI_Status status;
        MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vetChallenge_ask, MPI_COMM_WORLD, &status);
        int id = status.MPI_SOURCE;
        newStateNotification("We have been challenged to a duel from " + std::to_string(id));
        const std::lock_guard<std::mutex> guard(busy_m);
        if (bBusy)
        {
            char answer = false;
            MPI_Send(&answer, 1, MPI_CHAR, id, Message::vetChallenge_answer, MPI_COMM_WORLD);
            newStateNotification("Declining a duel from " + std::to_string(id));
        }
        else 
        {
            bBusy = true;
            char answer = true;
            MPI_Send(&answer, 1, MPI_CHAR, id, Message::vetChallenge_answer, MPI_COMM_WORLD);
            newStateNotification("Accepting a duel from " + std::to_string(id));
        }
	}
}