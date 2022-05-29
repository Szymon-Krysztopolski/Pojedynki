#include "Veteran.h"
#include "Second.h"
#include "Hospital.h"

unsigned						Second::W,					Second::S;

std::unique_ptr<int[]>			Second::secondingOnes,		Second::secondingOnesClock;

bool							Second::bBusy = false,		Second::bReserved = false,			Second::bWaiting_second = false;

int								Second::tID,				Second::reserved;

unsigned						Second::lamportClock;

std::unique_ptr<std::thread>	Second::answerSecond_th,	Second::answerVeteran_th,			Second::readiness_second_th,		Second::confirm_th,					Second::startWaiting_th;
std::mutex						Second::busy_m,				Second::waitingSecond_m,			Second::log_m;
std::condition_variable			Second::waitingSecond_cv;



unsigned						Veteran::W,					Veteran::S,							Veteran::SZ;

std::unique_ptr<int[]>			Veteran::fightingOnes,		Veteran::secondingOnes,				Veteran::fightingOnesClock,			Veteran::secondingOnesClock;


bool							Veteran::bBusy = false,		Veteran::bWaiting_veteran = false,	Veteran::bWaiting_second = false,	Veteran::bWounded = false;

int								Veteran::tID;

unsigned						Veteran::lamportClock;

std::unique_ptr<std::thread>	Veteran::challenge_th,		Veteran::answer_th,					Veteran::result_th,					Veteran::readiness_veterans_th,		Veteran::readiness_seconds_th,		Veteran::freeBed_th;
std::mutex						Veteran::challenge_m,		Veteran::busy_m,					Veteran::waiting_second_m,			Veteran::wounded_m,					Veteran::log_m,						Veteran::fighting_m,	Veteran::second_m;
std::condition_variable			Veteran::challenge_cv,		Veteran::second_cv;



std::unique_ptr<int[]>			Hospital::beds;
