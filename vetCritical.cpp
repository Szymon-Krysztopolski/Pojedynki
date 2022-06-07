#include "Veteran.h"

bool Veteran::criticalSection(bool bEnter)
{
	criticalSection_enter();
	bool freeBed = false;
	//If we are entering, we need to find a free bed 
	int i = 0;
	{
		std::lock_guard<std::mutex> lock(wounded_m);
		for (; i != SZ; ++i)
			if (hospitalBeds[i] == (bEnter ? -1 : tID))
			{
				//If entering, we set it to our ID, otherwise we free the bed (set to -1)
				hospitalBeds[i] = (bEnter ? tID : -1);
				//Currently leaving cannot fail, so it returns always true
				//Returning different values might be useful if we implemented queues
				freeBed = true;
				break;
			}
	}
	criticalSection_leave(i, bEnter);
	//We should always be able to find our bed if we toook one, but not being able to find a free bed returns false
	return freeBed;
}

void Veteran::criticalThinking()
{
	newStateNotification("Critical thread started");
	MPI_Status status;
	while (true)
	{
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, critical_Comm, &status);
		switch (status.MPI_TAG)
		{
			case vetHospitalDoor_open:
			{
				std::lock_guard<std::mutex> lck(door_m);
				hospital_open();
			}
		break;
			case vetHospitalDoor_close:
			{
				std::lock_guard<std::mutex> lock(door_m);
				std::lock_guard<std::mutex> lck(lamport_m);
				hospital_close();
			}
				//Critical section waits untill all the doors are closed, so we notify the variable one door just was closed
				door_cv.notify_one();
		break;
			case vetHospitalLeave:
			{
				std::lock_guard<std::mutex> lock(wounded_m);
				std::lock_guard<std::mutex> lck(lamport_m);
				criticalSectionLeft();
			}
				//If we are bleeding out, we want to know if there might be a free bed
				wounded_cv.notify_one();
				//Another thread will check if we can enter the critical section in case we want to
				lamport_cv.notify_one();
		break;
			default:
				newStateNotification("Critical thread: unknown tag");
		break;
		}
	}
	//Notify the challenge thread we are no longer busy and it should check if we can challenge others again
	veteran_cv.notify_one();
}

void Veteran::hospital_open()
{
	MPI_Status status;
	int doorClock;
	recv_vetHospitalDoor_open(&status, &doorClock);
	if (hospitalDoorClock[status.MPI_SOURCE] < doorClock)
	{
		hospitalDoorClock[status.MPI_SOURCE] = doorClock;
		hospitalDoor[status.MPI_SOURCE] = true;
	}
}

void Veteran::hospital_close()
{
	MPI_Status status;
	int data[3];
	recv_vetHospitalDoor_close(&status, &data);
	if (hospitalDoorClock[status.MPI_SOURCE] < data[1])
	{
		hospitalDoorClock[status.MPI_SOURCE] = data[1];
		hospitalDoor[status.MPI_SOURCE] = false;
	}
	if (lamportClock[status.MPI_SOURCE] < data[2])
	{
		lamportClock[status.MPI_SOURCE] = data[2];
		lamport[status.MPI_SOURCE] = data[0];
	}
}


void Veteran::criticalSection_enter()
{
#ifdef DEBUG
	newStateNotification("Critical entering");
#endif
	++myLamport; //increase the clock
	//Open the door
	for (unsigned i = 0u; i != W; ++i)
		if (i != tID)
			send_vetHospitalDoor_open(i, ++hospitalDoorClock[tID]);
	
	{
		std::lock_guard<std::mutex> lock(lamport_m);
		//Get maximum of tickers and set ours to max + 1
		int max = 0;
		for (unsigned i = 0u; i != W; ++i)
			if (lamport[i] > max)
				max = lamport[i];
		lamport[tID] = max + 1;
		//Close the door, informing of our ticket
		int data[3] = { lamport[tID], ++hospitalDoorClock[tID], ++lamportClock[tID] };
		for (unsigned i = 0u; i != W; ++i)
			if (i != tID)
				send_vetHospitalDoor_close(i, data);
	}
#ifdef DEBUG
	newStateNotification("lamport_m out");
#endif
	//Waiting until all the doors are closed
	{
		std::unique_lock<std::mutex> lck(door_m);
		door_cv.wait(lck, [] {
			bool bEmptyRoom = true;
			for (unsigned i = 0; i != W; ++i)
				if (i != tID)
					if (hospitalDoor[i])
					{
						bEmptyRoom = false;
						break;
					}
			return bEmptyRoom;
			});
	}
#ifdef DEBUG
	newStateNotification("door_m out");
#endif
	//Wait until it is our turn to enter the critical section
	std::unique_lock<std::mutex> lock(lamport_m);
	lamport_cv.wait(lock, []
		{
			bool myTurn = true;
			for (int i = 0; i != W; ++i)
				if (i != tID)
					if (lamport[i] > 0 && (lamport[tID] > lamport[i] || ((lamport[tID] == lamport[i]) && (tID > i))))
					{
						myTurn = false;
						break;
					}

			return myTurn;
		});
#ifdef DEBUG
	newStateNotification("Critical entered");
#endif
}

bool Veteran::criticalSection_leave(int hospitalBed, bool bEnter)
{
	lamport[tID] = 0; //discarding the ticket

	//Sending information to every Veteran that we have left the critical section and what we have done there
	std::lock_guard<std::mutex> lock1(wounded_m);
	std::lock_guard<std::mutex> lock2(lamport_m);
	int data[4] = { hospitalBed, bEnter, ++hospitalBedsClock[hospitalBed], ++lamportClock[tID] };
	for (unsigned i = 0u; i != W; ++i)
		if (i != tID)
			send_vetHospitalLeave(i, data);
#ifdef DEBUG
	newStateNotification("Critical left");
#endif
	return true;
}

void Veteran::criticalSectionLeft()
{
	MPI_Status status;
	int data[4];
	recv_vetHospitalLeave(&status, &data);
	++myLamport;
	//They make sure they found the right bed, because if they are entering, they might just not see a free bed and leave the critical section
	if (data[0] != -1)
	{
		//data[1] is a boolen whether they took the bed or freed it
		{
			if (data[1])
			{
				//Update bed status if another process didn't give us more recent information already
				//The bed is taken
				if (hospitalBedsClock[data[0]] < data[2])
				{
					hospitalBedsClock[data[0]] = data[2];
					hospitalBeds[data[0]] = status.MPI_SOURCE;
				}
			}
			else
			{
				//Update bed status if another process didn't give us more recent information already
				//The bed is freed	
				if (hospitalBedsClock[data[0]] < data[2])
				{
					hospitalBedsClock[data[0]] = data[2];
					hospitalBeds[data[0]] = -1;
				}

			}
		}
	}
	//Discard the ticket
	if (lamportClock[status.MPI_SOURCE] < data[3])
	{
		lamportClock[status.MPI_SOURCE] = data[3];
		lamport[status.MPI_SOURCE] = 0;
	}
}