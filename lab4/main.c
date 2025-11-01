#include <stdio.h>
#include <mpi.h>

int main(int argc, char ** argv) {
    int myrank, nprocs, len;
    char name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Get_processor_name(name, &len);

    printf("Hello from processor %s[%d] %d of %d\n", name, len, myrank, nprocs);

    if (myrank % 2 == 0) {
        if (myrank + 1 < nprocs) {
            int matrix[5][5];
            int *b = matrix[0];

            for (int i = 0; i < 25; i++) {
                *(b++) = i + 1;
            }

            MPI_Send(matrix, 25, MPI_INT, myrank + 1, 10, MPI_COMM_WORLD);
            printf("procs[%d] sent such matrix to procs[%d]:\n", myrank, myrank + 1);

            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    printf("%4d", matrix[i][j]);
                }
                printf("\n");
            }
            printf("\n");
        }
    } else {
        int matrix[5][5];

        MPI_Status st;
        MPI_Datatype vectorType, structType;

        MPI_Aint address[2];
        MPI_Datatype newType[2];
        int fieldsLen[2];

        MPI_Type_vector(5, 1, 5, MPI_INT, &vectorType);

        address[0] = 0;
        address[1] = sizeof(int);

        newType[0] = vectorType;
        newType[1] = MPI_UB;

        fieldsLen[0] = 1;
        fieldsLen[1] = 1;

        MPI_Type_create_struct(2, fieldsLen, address, newType, &structType);
        MPI_Type_commit(&structType);
        MPI_Type_free(&vectorType);

        MPI_Recv(matrix, 5, structType, myrank - 1, 10, MPI_COMM_WORLD, &st);
        printf("procs[%d] received transposed matrix from procs[%d]\nHere it is:\n", myrank, myrank - 1);

        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                printf("%4d", matrix[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        MPI_Type_free(&structType);
    }

    MPI_Finalize();
    return 0;
}