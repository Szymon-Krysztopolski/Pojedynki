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
	vet_ready,
	vet_challenge,
	vet_answer_challenge,
	vet_sec_ask,
	vet_duel_results,
	vet_bed_freed,

	sec_ready,
	sec_vet_answer,
	sec_vet_ask,
	sec_answer,
	sec_confirm
};

inline void send_readiness(int t_id, int* arr, Message message, int receiver_size, int who, int with_who = -1)
{
	int data[2] = { who, with_who };
	arr[who] = with_who;
	if (with_who != -1)
		arr[with_who] = arr[who];

	for (unsigned u = 0; u != receiver_size; ++u)
		if (u != t_id)
			MPI_Send(data, 2, MPI_INT, u, message, MPI_COMM_WORLD);
}

inline void readiness(int t_id, int *arr, Message message, void (*log_function)(std::string),  std::function<void(void)> lambda)
{
	int data[2];
	MPI_Status status;
	while (true)
	{
		MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, message, MPI_COMM_WORLD, &status);

		arr[data[0]] = data[1];
		lambda();

		std::string log = "Updating readiness of " + std::to_string(data[0]) + " who is now ";
		log += ((data[1] == -1) ? "ready" : "busy");
		log_function("Updating readiness of " + std::to_string(data[0]) + " who is now " + ((data[1] == -1) ? "ready" : "busy") 
			+ ((data[1] != -1) ? (" and " + std::to_string(data[1]) + " who is now also busy") : ""));
	}
}

inline bool find_free(int* arr, int maxSize, unsigned& random_free)
{
	std::vector<int> freeOnes;
	freeOnes.reserve(maxSize);

	for (unsigned i = 0; i != maxSize; ++i)
		if (arr[i] == -1)
			freeOnes.push_back(i);

	//todo: lepsze PRNG
	if (freeOnes.empty())
		return false;

	random_free = rand() % freeOnes.size();
	return true;
}