#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define REQ 100
#define RELEASE 200
#define ACK 300

int main(int argc, char **argv)
{
    int size, tid;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);

    if(size<3){
        printf("to few processes\n");
        MPI_Finalize();
        exit(0);
    }
    
    srand(tid);
    
    int mode=(rand()%3)%2; //0 -> Sekundant, 1 -> Weteran
    int SZ=size; //liczba lozek = liczbie procesów (pozniej zmienic)
    int lamport_timer=0;
    int vector_timer[size];
    for(int i=0;i<size;i++)vector_timer[i]=0;

    printf("proc: %d mode: %d\n",tid,mode);
    if(mode==0){
        for(int i=0;i<size;i++){
            ;//MPI_Send(&lamport_timer,1,MPI_INT,i,REQ,MPI_COMM_WORLD);
        }
    } else {
        ;
    }

    MPI_Finalize();
    return 0;
}
