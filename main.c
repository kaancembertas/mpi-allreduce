#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int Read_number(int p, int my_rank);
void Show_local_sums(int p, int my_rank,int sums);
void Apply_Allreduce (int p, int my_rank, int *local_sum);

int main(int argc, char *argv[])
{
    int my_rank;      /* rank of process      */
    int p;            /* number of processes  */
    int local_number; /* number for process   */
    int local_sum;    /* sum for a process    */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    local_number = Read_number(p, my_rank);
    local_sum = local_number;

    //MPI_Allreduce(&local_number,&local_sum,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
    Apply_Allreduce(p,my_rank,&local_sum);

    Show_local_sums(p,my_rank,local_sum);
    MPI_Finalize();
    return 0;
}

int Read_number(int p, int my_rank)
{
    int number_buf, local_number, dest;
    int tag = 0;
    MPI_Status status;

    if (my_rank == 0)
    {
        printf("Enter number for process 0: ");
        scanf("%d", &local_number);

        for (dest = 1; dest < p; dest++)
        {
            printf("Enter number for process %d: ", dest);
            scanf("%d", &number_buf);

            MPI_Send(&number_buf, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }
        return local_number;
    }

    MPI_Recv(&local_number, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    return local_number;
}

void Show_local_sums(int p, int my_rank,int local_sum)
{
    int buf, source;
    int dest = 0;
    int tag = 0;
    MPI_Status status;

    if(my_rank == 0)
    {
        printf("Process %d : %d\n",my_rank,local_sum);
        for(source=1; source<p; source++)
        {
            MPI_Recv(&buf,1,MPI_INT,source,tag,MPI_COMM_WORLD,&status);
            printf("Process %d: %d\n",source, buf);
        }
        return;
    }

    MPI_Send(&local_sum,1,MPI_INT,dest,tag,MPI_COMM_WORLD);
}

void Apply_Allreduce (int p, int my_rank, int *local_sum)
{
    /*
        STEP STARTS WITH   : 0
        DISTANCE FORMULA:  : p/(2^(step+1))
        SOURCE FORMULA     : (my_rank-distance+p) % p
        DESTINATION FORMULA: (my_rank+distance) % p 
    */

    int step, distance, dest,source,buf;
    int totalStep = (int)sqrt(p);

    MPI_Status status;

    for(step=0; 1 ; step++){
        distance = (int)(p / pow(2,step+1));
        source = (my_rank-distance+p) % p;
        dest = (my_rank+distance) % p;

        MPI_Send(local_sum,1,MPI_INT,dest,0,MPI_COMM_WORLD);
        MPI_Recv(&buf,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
        *local_sum += buf;
        
        if(distance==1) break;
    }
}
