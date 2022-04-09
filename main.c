#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MODE_MESS 50
#define REQ_r4f 110 // ready 4 fight
#define RELEASE 200
#define ACK_r4f 310
#define ACK_decf 320 // decision fight (accept||refuse)

int main(int argc, char **argv)
{
    int size, tid;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);

    if (size < 3)
    {
        printf("to few processes\n");
        MPI_Finalize();
        exit(0);
    }

    srand(tid);

    int mode = (rand() % 3) % 2; // 0 -> Sekundant, 1 -> Weteran
    int SZ = size;               // liczba lozek = liczbie procesów (pozniej zmienic)
    int lamport_timer = 0;
    int vector_timer[size], vector_modes[size];
    int loop_timer = 1; // how many loops

    for (int i = 0; i < size; i++)
    {
        vector_timer[i] = 0; // reset lamport_timer for everyone
        // MPI_SEND(skąd,ile,typ,do kogo,z jakim tagiem,MPI_COMM_WORLD);
        MPI_Send(&mode, 1, MPI_INT, i, MODE_MESS, MPI_COMM_WORLD); // send everyone who I am
    }
    // different loop because Recv is a block operation
    for (int i = 0; i < size; i++)
    {
        // MPI_Recv(gdzie,ile,jakiego typu,od kogo,z jakim tagiem,MPI_COMM_WORLD, &status);
        MPI_Recv(vector_modes + i, 1, MPI_INT, i, MODE_MESS, MPI_COMM_WORLD, &status);
    }

    int ready=1; //(mode==1) ready4fight || ready4help (mode==0)
    int yes=1;
    int no=0;

    //--------------------------------------------------------------------------------------
    printf("proc: %d mode: %d\n", tid, mode);
    while(loop_timer){
        if(ready){
            if(mode == 0){
                int sparring_partner=-1,ans;

                for(int i = 0; i < size; i++){
                    if(vector_modes[i]==0) {
                        MPI_Send(&lamport_timer,1,MPI_INT,i,REQ_r4f,MPI_COMM_WORLD);
                    }
                }
                
                for(int i = 0; i < size; i++){
                    if(vector_modes[i]==0) {
                        MPI_Recv(&ans,1,MPI_INT,i,ACK_r4f,MPI_COMM_WORLD,&status);

                        if(ans==1){
                            if(sparring_partner!=-1){
                                sparring_partner=i;
                                MPI_Send(&yes,1,MPI_INT,i,ACK_decf,MPI_COMM_WORLD);
                            } else {
                                MPI_Send(&no,1,MPI_INT,i,ACK_decf,MPI_COMM_WORLD);
                            }
                        }
                    }
                }
            } else {
                ;
            }
        }
        loop_timer--;
    }

    // test
    // if(tid==0)for(int i=0;i<size;i++) printf("%d ",vector_modes[i]);

    //--------------------------------------------------------------------------------------
    MPI_Finalize();
    return 0;
}
