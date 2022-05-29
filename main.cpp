#include "global.h"
#include "Veteran.h"
#include "Second.h"
#include "Hospital.h"

int main(int argc, char **argv)
{
    int size, t_id;
    srand(static_cast<unsigned int>(time(NULL)));
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &t_id);
    if (provided < MPI_THREAD_MULTIPLE)
    {
        printf("The threading support level is lesser than that demanded.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (argc < 4)
    {
        std::cerr << "Not enough arguments" << std::endl;
        return 0;
    }
    int W = atoi(argv[1]), S = atoi(argv[2]), SZ = atoi(argv[3]);

    if (W < 2
#ifndef NO_SECONDS_DEBUG
        || (S < 2)
#endif     
        )
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
        Hospital::init(SZ);
        Veteran::init(t_id, W, S, SZ);
        Veteran::start();
        Veteran::join();
        Hospital::clean();
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
