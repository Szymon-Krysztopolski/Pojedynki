#pragma once
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

inline void sendReadiness(int t_id, int* arr, Message message, int receiver_size, int who, int with_who = -1)
{
	int data[2] = { who, with_who };
	arr[who] = with_who;
	if (with_who != -1)
		arr[with_who] = arr[who];

	for (unsigned u = 0; u != receiver_size; ++u)
		if (u != t_id)
			MPI_Send(data, 2, MPI_INT, u, message, MPI_COMM_WORLD);
}

inline void readiness(int tID, int *arr, Message message, void (*logFunction)(std::string),  std::function<void(void)> lambda)
{
	int data[2];
	MPI_Status status;
	while (true)
	{
		MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, message, MPI_COMM_WORLD, &status);

		arr[data[0]] = data[1];
		lambda();

		logFunction("Updating readiness of " + std::to_string(data[0]) + " who is now " + ((data[1] == -1) ? "ready" : "busy") 
			+ ((data[1] != -1) ? (" and " + std::to_string(data[1]) + " who is now also busy") : ""));
	}
}

inline bool findFree(int* arr, int maxSize, unsigned& randomFree, bool bOnlyOlder = false, unsigned tID = 0u)
{
	std::vector<int> freeOnes;
	freeOnes.reserve(maxSize);

	for (unsigned i = (bOnlyOlder ? tID : 0u); i != maxSize; ++i)
		if (arr[i] == -1)
			freeOnes.push_back(i);

	//todo: lepsze PRNG
	if (freeOnes.empty())
		return false;

	randomFree = rand() % freeOnes.size();
	return true;
}