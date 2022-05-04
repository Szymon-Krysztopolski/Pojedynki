#include "global.h"
#include "Veteran.h"
#include "Second.h"
#include "Hospital.h"

//static variables... todo: maybe shove them somewhere else for better clarity
unsigned    Second::W, Second::S;

std::unique_ptr<int[]>   Second::seconding_ones;

bool        Second::b_busy, Second::b_reserved, Second::b_waiting_sec;

int         Second::t_id;

unsigned    Second::lamport_clock;

std::unique_ptr<std::thread> Second::answer_vet_th, Second::answer_sec_th, Second::sec_readiness_th, Second::confirm_th;
std::mutex Second::m_busy, Second::m_waiting_sec, Second::m_log;

unsigned Veteran::W, Veteran::S, Veteran::SZ;
std::unique_ptr<int[]>      Veteran::fighting_ones,
Veteran::seconding_ones;

bool        Veteran::b_busy, Veteran::b_waiting_vet, Veteran::b_waiting_sec, Veteran::b_wounded;

int         Veteran::t_id;

unsigned    Veteran::lamport_clock;

std::unique_ptr<std::thread> Veteran::challenge_th, Veteran::answer_th, Veteran::result_th, Veteran::vet_readiness_th, Veteran::sec_readiness_th, Veteran::free_bed_th;
std::mutex Veteran::m_challenge, Veteran::m_busy, Veteran::m_waiting_free_sec, Veteran::m_waiting_sec, Veteran::m_wounded, Veteran::m_log;

int main(int argc, char **argv)
{
    int size, t_id;
    srand(static_cast<unsigned int>(time(NULL)));
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &t_id);

    if (argc < 4)
    {
        std::cerr << "Not enough arguments" << std::endl;
        return 0;
    }
    int W = atoi(argv[1]), S = atoi(argv[2]), SZ = atoi(argv[3]);

    if (W < 2 || S < 2)
    {
        std::cerr << "Not enough veterans or seconds to hold a proper, honorable duel for the glory of us all" << std::endl;
        return 0;
    }
    if (size != (W + S))
    {
        std::cerr << "Declared and actual people missmatch. Make sure W and S are set properly" << std::endl;
        return 0;
    }


    if (t_id < W)
    {
        Veteran::init(t_id, W, S, SZ);
        Veteran::start();
        Veteran::join();
    }
    else if (t_id < W + S)
    {
        Second::init(t_id, W, S);
        Second::start();
        Second::join();
    }
    
    MPI_Finalize();
    return 0;
}
