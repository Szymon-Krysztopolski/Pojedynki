#pragma once
#include "global.h"

class Hospital
{
public:
	static void init(unsigned SZ)
	{
		beds = std::unique_ptr<int[]>(new int [SZ]);
		memset(beds.get(), -1, SZ);
	}
	static void clean()
	{
		beds = nullptr;
	}
	static std::unique_ptr<int[]> beds;
};