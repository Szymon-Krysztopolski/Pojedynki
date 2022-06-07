#include "global.h"
#include "Veteran.h"
#include "Second.h"

int main(int argc, char **argv)
{
    int provided, size, t_id;
    srand(static_cast<unsigned int>(time(NULL)));
    //Our development technique is quite odd, because we separated every MPI_TAG's MPI_Receive into a different thread
    //Pros: 
    //  better usage of logical cores
    //  easier debugging and maintaining code (at least for us)
    //  no need to probe
    //  no need to create new communication channels
    //  easier to get rid of pooling using mutexes with conditional variables
    //  more bandwitch
    //Cons:
    //  need to protect variables by mutexes
    //  cpu overheads from context switches and mutex management
    //  cannot fully utilize FIFO nature of the channel (it is not an issue in our assigment otherwise we would never use this technique)
    //Maybe aggregation of some those channels and creating communication channels is proper thing to do, but we wanted
    //to give this one a try nonetheless and it works quite good
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

    //Checking if the conditions are valid
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
    if (SZ < 1)
    {
        std::cerr << "Not having a hospital bed is against safety policies. We cannot continue (without legal issues)" << std::endl;
        return 0;
    }

    //MPI_Comm_split(MPI_COMM_WORLD, t_id < W, t_id, &local_Comm);
    MPI_Comm_dup(MPI_COMM_WORLD, &local_Comm);
    MPI_Comm_dup(MPI_COMM_WORLD, &readiness_Comm);

    //The logical distinguishment between Veteran and Second 
    //Yes, they are "static" classes, meaning we made their every field and method static
    //Just to ensure no one will create it twice
    //Small downside is that the other class has allocated a never used memory
    //But who cares about a kilobyte on a modern PC
    if (t_id < W)
    {
        MPI_Comm_dup(local_Comm, &chall_Comm);
        MPI_Comm_dup(local_Comm, &critical_Comm);
        Veteran::init(t_id, W, S, SZ);
        Veteran::start();
        Veteran::join();
    }
    else if (t_id < W + S)
    {
        MPI_Comm_dup(local_Comm, &waiting_Comm);
        Second::init(t_id, W, S);
        Second::start();
        Second::join();
    }
    
    MPI_Finalize();
    return 0;
}
