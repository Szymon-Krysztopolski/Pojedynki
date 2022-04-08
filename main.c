#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    srand(time(NULL));
    int size, tid;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);
    
    int SZ=size; //liczba lozek = liczbie procesów (pozniej zmienic)
    int mode=(rand()%3)%2; //0 -> Sekundant, 1 -> Weteran

    printf("proc: %d mode: %d",tid,size);

    MPI_Finalize();
    return 0;
}