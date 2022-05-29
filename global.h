#pragma once
//#define NO_SECONDS_DEBUG
#define DEBUG
#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include <time.h>
#include <thread>
#include <mutex>
#include <functional>

enum Message
{
	vetReady,
	vetChallenge_ask,
	vetChallenge_answer,
	vetSecond_ask,
	vetSecond_answer,
	vetResults,
	vetBedFreed,

	secReady,
	secSecond_ask,
	secSecond_answer,
	secConfirm,
	secNoFreeSecond
};

/*
inline void sendReadiness(int tID, int *arr, int *clockArr, Message message, int receiver_size, int who, int with_who = -1, bool bTwoSide = true)
{
	arr[who]			= with_who;
	++clockArr[who];
	if (bTwoSide && with_who != -1)
	{
		++clockArr[with_who];
		arr[with_who]	= who;
	}

	int data[4] = { who, with_who, clockArr[who], (bTwoSide && with_who != -1) ? clockArr[with_who] : 0 };
	for (unsigned u = 0; u != receiver_size; ++u)
		if (u != tID)
			MPI_Send(data, 4, MPI_INT, u, message, MPI_COMM_WORLD);
}*/

inline void sendReadiness(int tID, int* arr, int* clockArr, Message message, int receiver_size, int who, int with_who = -1, int add = 0)
{
	arr[who] = with_who;

	int data[3] = { who, with_who, clockArr[who] };
	if ((who + add) == tID)
	{
		data[2] = ++clockArr[who];
		for (unsigned u = 0; u != receiver_size; ++u)
			if (u != tID)
				MPI_Send(data, 3, MPI_INT, u, message, MPI_COMM_WORLD);
	}
	else
		MPI_Send(data, 3, MPI_INT, who + add, message, MPI_COMM_WORLD);

}

inline void readiness(int tID, int* arr, int* clockArr, Message message, int receiver_size, void (*logFunction)(std::string), std::function<void(void)> lambda, int add = 0)
{
	MPI_Status status;
	while (true)
	{
		int data[3];
		MPI_Recv(data, 3, MPI_INT, MPI_ANY_SOURCE, message, MPI_COMM_WORLD, &status);

		if (data[1] == -1 || clockArr[data[0]] <= data[2])
		{
			arr[data[0]] = data[1];
			clockArr[data[0]] = data[2];
		}

		if ((data[0] + add) == tID)
		{
			for (unsigned u = 0; u != receiver_size; ++u)
				if (u != tID)
					MPI_Send(data, 3, MPI_INT, u, message, MPI_COMM_WORLD);
		}
		lambda();
#ifdef DEBUG
		logFunction("Updating readiness of " + std::to_string(data[0] + add) + "[" + std::to_string(data[2]) + "] who is now " + ((data[1] == -1) ? "ready" : "busy"));
#endif
	}
}

/*
inline void readiness(int tID, int arrID, int *arr, int *clockArr, Message message, int receiver_size, void (*logFunction)(std::string),  std::function<void(void)> lambda, bool bTwoSide = true, int twoSideAdd = 0)
{  
	MPI_Status status;
	while (true)
	{
		int data[4];
		MPI_Recv(data, 4, MPI_INT, MPI_ANY_SOURCE, message, MPI_COMM_WORLD, &status);

		//When someone messaged about readiness but our state has changed
		//if (arrID == data[0] && data[1] == -1 && arr[arrID] != -1 && clockArr[arrID] >= data[2])
		//{
		//	data[2] = clockArr[arrID];
		//	data[1] = arr[data[0]];
		//	std::cout <<"vibe "<< tID << std::endl << std::flush;
		//	MPI_Send(data, 4, MPI_INT, status.MPI_SOURCE, message, MPI_COMM_WORLD);
		//	continue;
		//}
		//else
		if (clockArr[data[0]] < data[2])
		{
			arr[data[0]]		= data[1];
			clockArr[data[0]]	= data[2];
		}

		if (bTwoSide && (data[1] != -1) && (clockArr[data[1]] < data[3]))
		{
			arr[data[1]]		= data[0];
			clockArr[data[1]]	= data[3];
		}
		lambda();
#ifdef DEBUG
		logFunction("Updating readiness of " + std::to_string(data[0] + twoSideAdd) + "[" + std::to_string(data[2]) + "] who is now " + ((data[1] == -1) ? "ready" : "busy")
			+ ((bTwoSide && (data[1] != -1)) ? (" and " + std::to_string(data[1]) + "[" + std::to_string(data[3]) + "] who is now also busy") : ""));
#endif
	}
}*/



inline bool findFree(int* arr, int maxSize, unsigned& randomFree, bool bOnlyOlder = false, int tID = -1)
{
	std::vector<int> freeOnes;
	freeOnes.reserve(maxSize);

	for (int i = (bOnlyOlder ? (tID + 1) : 0u); i != maxSize; ++i)
		if (arr[i] == -1 && i != tID)
			freeOnes.push_back(i);

	//todo: lepsze PRNG
	if (freeOnes.empty())
		return false;

	randomFree = freeOnes[rand() % freeOnes.size()];
	return true;
}