#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MODE_MESS 50
#define REQ_r4f 110 //ready 4 fight
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
    int vector_timer[size],vector_modes[size];
    int loop_timer=1; //how many loops

    for(int i=0;i<size;i++){
        vector_timer[i]=0; // reset lamport_timer for everyone
        //MPI_SEND(skąd,ile,typ,do kogo,z jakim tagiem,MPI_COMM_WORLD);
        MPI_Send(&mode,1,MPI_INT,i,MODE_MESS,MPI_COMM_WORLD); // send everyone who I am
    }
    //different loop because Recv is a block operation
    for(int i=0;i<size;i++){
        //MPI_Recv(gdzie,ile,jakiego typu,od kogo,z jakim tagiem,MPI_COMM_WORLD, &status);
        MPI_Recv(vector_modes+i,1,MPI_INT,i,MODE_MESS,MPI_COMM_WORLD,&status);
    }

    //--------------------------------------------------------------------------------------
    printf("proc: %d mode: %d\n",tid,mode);
    while(loop_timer){
        if(mode==0){
            for(int i=0;i<size;i++){
                ;//MPI_Send(&lamport_timer,1,MPI_INT,i,REQ,MPI_COMM_WORLD);
            }
        } else {
            ;
        }
        loop_timer--;
    }

    //test
    //if(tid==0)for(int i=0;i<size;i++) printf("%d ",vector_modes[i]);

    //--------------------------------------------------------------------------------------
    MPI_Finalize();
    return 0;
}
