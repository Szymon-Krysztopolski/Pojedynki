#include "Veteran.h"

bool Veteran::criticalSection()
{
	enter_criticalSection();
	//work
	leave_criticalSection();
	return true;
}

void Veteran::enter_criticalSection()
{
	//todo:
}

void Veteran::leave_criticalSection()
{
	//todo:
}

void Veteran::endDuel_result(bool won)
{
	if (!won)
		if (!criticalSection())
			return;
	{
		const std::lock_guard<std::mutex> lock(busy_m);
		bBusy = false;
	}
	sendVeteranReadiness_toVeterans(tID);
	challenge_m.unlock();
}

void Veteran::result()
{
	MPI_Status status;
	char data;
	while (true)
	{
		MPI_Recv(&data, 1, MPI_CHAR, MPI_ANY_SOURCE, Message::vetResults, MPI_COMM_WORLD, &status);

		//cancelled duel
		if (data == 2)
		{
			newStateNotification("Challenging " + std::to_string(data) + " and waiting for their answer");
			{
				const std::lock_guard<std::mutex> lock(busy_m);
				bBusy = false;
			}
			challenge_m.unlock();
		}
		else endDuel_result(data);
	}
}

void Veteran::freeBed()
{
	MPI_Status status;
	while (true)
	{
		MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vetBedFreed, MPI_COMM_WORLD, &status);
		newStateNotification("A bed has been freed");
		{
			const std::lock_guard<std::mutex> lock(wounded_m);
			if (bWounded)
				endDuel_result(false);
		}
	}
}
