#include "Veteran.h"
#include "Second.h"
//Definitions of declared static variables

MPI_Comm						local_Comm,					readiness_Comm,						chall_Comm,							critical_Comm,						waiting_Comm;

unsigned						Second::W,					Second::S;

std::unique_ptr<int[]>			Second::secondingOnes,		Second::secondingOnesClock;

bool							Second::bBusy = false,		Second::bReserved = false;

int								Second::tID,				Second::reserved,					Second::agreed;

std::unique_ptr<std::thread>	Second::veteran_th,			Second::answer_th,					Second::readiness_th,				Second::waiting_th;
std::mutex						Second::secbusy_m,			Second::log_m;
std::condition_variable			Second::waitingSecond_cv;



unsigned						Veteran::W,					Veteran::S,							Veteran::SZ,						Veteran::myLamport = 0;

std::unique_ptr<int[]>			Veteran::fightingOnes,		Veteran::secondingOnes,				Veteran::fightingOnesClock,			Veteran::secondingOnesClock;


bool							Veteran::bBusy = false;

int								Veteran::tID;

std::unique_ptr<int[]>			Veteran::lamport,			Veteran::lamportClock,				Veteran::hospitalBeds,				Veteran::hospitalBedsClock,			Veteran::hospitalDoorClock;
std::unique_ptr<bool[]>			Veteran::hospitalDoor;



std::unique_ptr<std::thread>	Veteran::challenge_th,		Veteran::answer_th,					Veteran::readiness_th,				Veteran::critical_th;
std::mutex						Veteran::challenge_m,		Veteran::busy_m,					Veteran::wounded_m,					Veteran::log_m,						Veteran::second_m,					
								Veteran::lamport_m,			Veteran::door_m;
std::condition_variable			Veteran::veteran_cv,		Veteran::second_cv,					Veteran::lamport_cv,				Veteran::wounded_cv,				Veteran::door_cv;

