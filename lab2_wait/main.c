#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define SIZE 1024

int main(int argc, char ** argv)
{
    int myrank, nprocs, len;
    char name[MPI_MAX_PROCESSOR_NAME];
    int *buf1, *buf2, *buftmp;
    MPI_Status st;
    MPI_Request rq;

    buf1 = (int *)malloc(sizeof(int) * (SIZE * 1024 + 100));
    buf2 = (int *)malloc(sizeof(int) * (SIZE * 1024 + 100));
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Get_processor_name(name, &len);

    printf("Hello from processor %s[%d] %d of %d\n", name, len, myrank, nprocs);

    if ( myrank % 2 == 0 )
    {
        if ( myrank < nprocs - 1 )
        {
            int i, cl, sz = SIZE;
            double time;

            for ( i = 0; i < SIZE * 1024; i++ )
                buf1[i] = i + 10;

            for ( ; i < SIZE * 1024 + 100; i++ )
                buf1[i] = 0;

            for ( cl = 0; cl < 11; cl++ )
            {
                time = MPI_Wtime();

                for ( i = 0; i < 100; i++ )
                {
                    MPI_Irecv( buf2, sz + 100, MPI_INT, myrank + 1, 20, MPI_COMM_WORLD, &rq );
                    MPI_Ssend( buf1, sz, MPI_INT, myrank + 1, 10, MPI_COMM_WORLD );
                    MPI_Wait( &rq, &st );
                    
                    buftmp = buf1;
                    buf1 = buf2;
                    buf2 = buftmp;
                }

                time = MPI_Wtime() - time;
                printf( "[%d] Time = %lf  Data = %9.0f KByte\n", myrank, time, sz * sizeof(int) * 200.0 / 1024 );
                printf( "[%d] Bandwith[%d] = %lf MByte/sek\n", myrank, cl, sz * sizeof(int) * 200 / (time * 1024 * 1024) );
                sz *= 2;
            }
        }
        else
            printf("[%d] Idle\n", myrank);
    }
    else
    {
        int i, cl, sz = SIZE;

        for ( i = 0; i < SIZE * 1024; i++ )
            buf1[i] = i * 10 + 5;

        for ( ; i < SIZE * 1024 + 100; i++ )
            buf1[i] = 0;

        for ( cl = 0; cl < 11; cl++ )
        {
            for ( i = 0; i < 100; i++ )
            {
                MPI_Irecv( buf2, sz + 100, MPI_INT, myrank - 1, 10, MPI_COMM_WORLD, &rq );
                MPI_Ssend( buf1, sz, MPI_INT, myrank - 1, 20, MPI_COMM_WORLD );
                MPI_Wait( &rq, &st );

                if ( i > 97) {
                    printf("[%2d] %5d    %5d    %5d\n", myrank, buf2[0], buf2[1], buf2[2]);
                }

                buftmp = buf1;
                buf1 = buf2;
                buf2 = buftmp;
            }
            sz *= 2;
        }
    }

    MPI_Finalize();
    return 0;
}