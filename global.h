#pragma once
//Debug Veterans only. No need to ask for Seconds
//#define NO_SECONDS_DEBUG

//Print every readiness change (fightingOnes and secondingOnes and critical section enter/leave messages
//#define DEBUG
#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

#include <time.h>
#include <thread>
#include <mutex>
#include <functional>

//Duel time = DUEL_BASE_TIME + rand() % DUEL_RAND_TIME
#define DUEL_BASE_TIME 100
#define DUEL_RAND_TIME 40
//Healing (hospital) time = HEAL_BASE_TIME + rand() % HEAL_RAND_TIME
#define HEAL_BASE_TIME 200
#define HEAL_RAND_TIME 100

extern MPI_Comm local_Comm, readiness_Comm, 
//Vet
chall_Comm, critical_Comm, 
//Sec
waiting_Comm;

//All the tags used in program
enum Message
{
	vetReady,
	vetChallenge_ask,
	vetChallenge_answer,
	vetSecond_ask,
	vetSecond_answer,
	vetResults,
	vetHospitalDoor_open,
	vetHospitalDoor_close,
	vetHospitalLeave,
		
	secReady,
	secSecond_ask,
	secConfirm,
	secNoFreeSecond
};

//All MPI_Sends and Receives in one place, so we can see that the sent and received data types and size match
inline void send_vetChallenge_ask(unsigned who)
{
	MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::vetChallenge_ask, local_Comm);
}

inline void recv_vetChallenge_ask(MPI_Status *status)
{
	MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vetChallenge_ask, local_Comm, status);
}

//0: answer - accept or decline of their duel
inline void send_vetChallenge_answer(unsigned who, char data)
{
	MPI_Send(&data, 1, MPI_CHAR, who, Message::vetChallenge_answer, chall_Comm);
}

//0: answer - accept or decline of our duel
inline void recv_vetChallenge_answer(MPI_Status* status, char* data, int who)
{
	MPI_Recv(data, 1, MPI_CHAR, who, Message::vetChallenge_answer, chall_Comm, status);
}

inline void send_vetSecond_ask(unsigned who)
{
	MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::vetSecond_ask, MPI_COMM_WORLD);
}

inline void recv_vetSecond_ask(MPI_Status* status)
{
	MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vetSecond_ask, MPI_COMM_WORLD, status);
}

//0: boolean whether we accept or decline the offer to second in this duel
//1: ID of the second Second in case we accept, w/e otherwise
inline void send_vetSecond_answer(unsigned who, int data[2])
{
	MPI_Send(data, 2, MPI_INT, who, Message::vetSecond_answer, MPI_COMM_WORLD);
}

//0: boolean whether they accepted or declined the offer to second in our duel
//1: ID of the second Second in case they accept, w/e otherwise
inline void recv_vetSecond_answer(MPI_Status* status, int(*data)[2], int who)
{
	MPI_Recv(*data, 2, MPI_INT, who, Message::vetSecond_answer, MPI_COMM_WORLD, status);
}

//0: whether they won or lost the duel. There is also special value "2" for a cancellation of the duel
inline void send_vetResults(unsigned who, char data)
{
	MPI_Send(&data, 1, MPI_CHAR, who, Message::vetResults, local_Comm);
}

//0: whether we won or lost the duel. There is also special value "2" for a cancellation of the duel
inline void recv_vetResults(MPI_Status* status, char* data)
{
	MPI_Recv(data, 1, MPI_CHAR, MPI_ANY_SOURCE, Message::vetResults, local_Comm, status);
}

//0: clock of our door
inline void send_vetHospitalDoor_open(unsigned who, int data)
{
	MPI_Send(&data, 1, MPI_INT, who, Message::vetHospitalDoor_open, critical_Comm);
}

//0: clock of their door
inline void recv_vetHospitalDoor_open(MPI_Status* status, int* data)
{
	MPI_Recv(data, 1, MPI_INT, MPI_ANY_SOURCE, Message::vetHospitalDoor_open, critical_Comm, status);
}

//0: our ticket
//1: clock of their door
//2: our ticket's clock
inline void send_vetHospitalDoor_close(unsigned who, int data[3])
{
	MPI_Send(data, 3, MPI_INT, who, Message::vetHospitalDoor_close, critical_Comm);
}

//0: their ticket
//1: clock of their door
//2: their ticket's clock
inline void recv_vetHospitalDoor_close(MPI_Status* status, int (*data)[3])
{
	MPI_Recv(*data, 3, MPI_INT, MPI_ANY_SOURCE, Message::vetHospitalDoor_close, critical_Comm, status);
}

//0: the hospital bed that we modified
//1: a boolean whether we took the bed or freed it
//2: our clock for hospital beds
//3: our ticket clock (because implicitly we discard our ticket)
inline void send_vetHospitalLeave(unsigned who, int data[4])
{
	MPI_Send(data, 4, MPI_INT, who, Message::vetHospitalLeave, critical_Comm);
}

//0: the hospital bed that they modified
//1: a boolean whether they took the bed or freed it
//2: their clock for hospital beds
//3: their ticket clock (because implicitly they discard their ticket)
inline void recv_vetHospitalLeave(MPI_Status* status, int (*data)[4])
{
	MPI_Recv(*data, 4, MPI_INT, MPI_ANY_SOURCE, Message::vetHospitalLeave, critical_Comm, status);
}

//0: tID of the first Second that asked (asking flows in pipeline, if a Second we asked is busy, they will ask others in our name, so we need to send it first)
//1: ID of the Veteran hosting the duel, for debugging purposes and setting busy status to Veteran (which could be just a boolean, so it is useless too, but not for debugging)
inline void send_secSecond_ask(unsigned who, int data[2])
{
	MPI_Send(data, 2, MPI_INT, who, Message::secSecond_ask, local_Comm);
}

//0: tID of the first Second that asked (asking flows in pipeline, if a Second we asked is busy, they will ask others in our name, so we need to send it first)
//1: ID of the Veteran hosting the duel, for debugging purposes and setting busy status to Veteran (which could be just a boolean, so it is useless too, but not for debugging)
inline void recv_secSecond_ask(MPI_Status* status, int (*data)[2])
{
	MPI_Recv(*data, 2, MPI_INT, MPI_ANY_SOURCE, Message::secSecond_ask, local_Comm, status);
}

inline void send_secConfirm(unsigned who)
{
	MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::secConfirm, local_Comm);
}

inline void recv_secConfirm(MPI_Status* status)
{
	MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::secConfirm, local_Comm, status);
}

inline void send_secNoFreeSecond(unsigned who)
{
	MPI_Send(nullptr, 0, MPI_DATATYPE_NULL, who, Message::secNoFreeSecond, waiting_Comm);
}

inline void recv_secNoFreeSecond(MPI_Status* status)
{
	MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::secNoFreeSecond, waiting_Comm, status);
}

//Generalization of sendReadiness
inline void sendReadiness(int tID, int* arr, int* clockArr, Message message, int receiver_size, int who, int with_who = -1, int add = 0)
{
	//Set the readiness in our local structure first
	//The table maps a thread ID "who" to another thread ID "with_who", meaning "who" is busy with "with_who"
	//Yes, it could be boolean only array, but debugging this way is easier
	//And we wanted to visualize this in QT, but sadly no time :/

	//Data to be sent:
	//0:  ID of the one that is becoming busy
	//1:  ID of the one that the busy one is busy with
	//2:  Lamport clock. It is kind of protecting the variable from being overwritten by message from other process that has not actual information
	//    It can only happen when sending a state change (because otherwise only the proces that changes readiness state sends it to the others, which cannot fail because it is FIFO)
	int data[3] = { who, with_who, clockArr[who] };
	if ((who + add) == tID)
	{
		//The debug state '-2' of Veteran can race with its free state, so here we prioritize free over wounded
		if (with_who == -2 && arr[who] == -1)
			data[1] = -1;
		data[2] = ++clockArr[who];
		//Inform others my (== tID) state changed 
		arr[who] = with_who;
		for (int i = 0; i != receiver_size; ++i)
			if (i != tID)
				MPI_Send(data, 3, MPI_INT, i, message, readiness_Comm);
	}
	else
	{
		arr[who] = with_who;
		//Inform someone their (!= tID) state changed
		//Under special circuimtances (usually too small sleep time) this can be done in wrong time and this is why clocks are imporant
		//Yes, we could send entire table and protect it with a single clock, but why be inefficient?
		MPI_Send(data, 3, MPI_INT, who + add, message, readiness_Comm);
	}
}

//Generalization of the readiness thread
inline void readiness(int tID, int* arr, int* clockArr, Message message, int receiver_size, void (*logFunction)(std::string), int offset = 0)
{
	MPI_Status status;
	int data[3];
	MPI_Recv(data, 3, MPI_INT, MPI_ANY_SOURCE, message, readiness_Comm, &status);
	{
		//Checking the clock
		//Sending a message to set someone to be free must be a most recent (in terms of clock) action, but the process that sent us this message might not have our most recent state
		//This is also the reason why we are taking max belowe
		if (data[1] == -1 || clockArr[data[0]] < data[2])
		{
			arr[data[0]] = data[1];
			clockArr[data[0]] = std::max(clockArr[data[0]], data[2]);
		}
		//We received our state changed. Now we need to tell others
		//The process that changed our state cannot send it to others directly, we must get the information first
		//Otherwise other processes could think we (process with current tID) are ready when we in fact are ready, but think we are not
		//And the interaction would be invalid until we get the message of our readiness
		//tID does not always correspond directly with the index in the array, so sometimes we need to add an offset
		if ((data[0] + offset) == tID)
		{
			for (unsigned u = 0; u != receiver_size; ++u)
				if (u != tID)
					MPI_Send(data, 3, MPI_INT, u, message, readiness_Comm);
		}
	}
#ifdef DEBUG
	logFunction("Updating readiness of " + std::to_string(data[0] + offset) + "[" + std::to_string(data[2]) + "] who is now " + ((data[1] == -1) ? "ready" : "busy"));
#endif
}

//Tries to finds a nonbusy process in the array and saves a random one of the available in randomFree. Returns false on failure
inline bool findFree(int* arr, int maxSize, unsigned *randomFree, bool bOnlyOlder = false, int tID = -1)
{
	std::vector<int> freeOnes;
	freeOnes.reserve(maxSize); //allocates memory beforehand. Optimization 

	//Add all free indexes from arr to freeOnes. Do not add yourself (tID)
	//bOnlyOlder does as the name suggests: only indexes of processes with higher tID are added
	for (int i = (bOnlyOlder ? (tID + 1) : 0); i != maxSize; ++i)
		if (arr[i] == -1 && i != tID)
			freeOnes.push_back(i);

	if (freeOnes.empty())
		return false;

	//Pick an index at random
	*randomFree = freeOnes[rand() % freeOnes.size()];
	return true;
}

