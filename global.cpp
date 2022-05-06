#include "Veteran.h"
#include "Second.h"
#include "Hospital.h"

unsigned    Second::W, Second::S;

std::unique_ptr<int[]>   Second::secondingOnes;

bool        Second::bBusy, Second::bReserved, Second::bWaiting_second;

int         Second::tID, Second::reserved;

unsigned    Second::lamportClock;

std::unique_ptr<std::thread> Second::answerSecond_th, Second::answerVeteran_th, Second::readiness_second_th, Second::confirm_th, Second::startWaiting_th;
std::mutex Second::busy_m, Second::waitingSecond_m, Second::log_m;

unsigned Veteran::W, Veteran::S, Veteran::SZ;
std::unique_ptr<int[]>      Veteran::fightingOnes,
Veteran::secondingOnes;

bool        Veteran::bBusy, Veteran::bWaiting_veteran, Veteran::bWaiting_second, Veteran::bWounded;

int         Veteran::tID;

unsigned    Veteran::lamportClock;

std::unique_ptr<std::thread> Veteran::challenge_th, Veteran::answer_th, Veteran::result_th, Veteran::readiness_veterans_th, Veteran::readiness_seconds_th, Veteran::freeBed_th;
std::mutex Veteran::challenge_m, Veteran::busy_m, Veteran::waiting_untilFreeSecond_m, Veteran::waiting_second_m, Veteran::wounded_m, Veteran::log_m;

std::unique_ptr<int[]> Hospital::beds;
