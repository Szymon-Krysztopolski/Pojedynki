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
		std::lock_guard<std::mutex> lock(busy_m);
		unsetBusy();
	}
	sendVeteranReadiness_toVeterans(tID);
	challenge_cv.notify_one();
}

void Veteran::result()
{
	newStateNotification("Result thread started");

	MPI_Status status;
	char data;
	while (true)
	{
		MPI_Recv(&data, 1, MPI_CHAR, MPI_ANY_SOURCE, Message::vetResults, MPI_COMM_WORLD, &status);

		//cancelled duel
		if (data == 2)
		{
			//MPI_Abort(MPI_COMM_WORLD, 15);
			newStateNotification("A duel with " + std::to_string(data) + " has been cancelled");
			{
				std::lock_guard<std::mutex> lock(busy_m);
				unsetBusy();
			}
			challenge_cv.notify_one();
		}
		else endDuel_result(data);
	}
}

void Veteran::freeBed()
{
	newStateNotification("FreeBed thread started");

	MPI_Status status;
	while (true)
	{
		MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vetBedFreed, MPI_COMM_WORLD, &status);
		newStateNotification("A bed has been freed");
		{
			std::lock_guard<std::mutex> lock(wounded_m);
			if (bWounded)
				endDuel_result(false);
		}
	}
}
