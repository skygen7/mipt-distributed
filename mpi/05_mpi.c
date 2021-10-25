#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void mpi_check(int mpi_type) {
    if (mpi_type != MPI_SUCCESS) {
        printf("Erorr in MPI\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_type);
    }
}


int main(int argc, char *argv[]) {
    int size, rank;
    MPI_Status st;
    mpi_check(MPI_Init(&argc, &argv));

    mpi_check(MPI_Comm_size(MPI_COMM_WORLD, &size));

    mpi_check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    // Задаем начальное значение генератора случайных величин
    srand(rank + 1);

    if (rank == 0) {
        // Server time
        long int utc = rand()%15;
        printf("Start server time: %lu\n", utc);
        for (int i = 1; i < size - 1; i++) {
            long int client_time;
            mpi_check(MPI_Recv(&client_time, 1, MPI_LONG_INT, i, 1, MPI_COMM_WORLD, &st));
            // correct client time
            mpi_check(MPI_Send(&utc, 1, MPI_LONG_INT, i, 1, MPI_COMM_WORLD));
            printf("Send to %d UTC: %lu\n", i, utc);
            utc += rand()%15;
        }
    } else {
        long int client_time0 = rand()%15;
        long int utc;
        // send to server
        mpi_check(MPI_Send(&client_time0, 1, MPI_LONG_INT, 0, 1, MPI_COMM_WORLD));
        printf("Process: %d. Local time: %lu\n", rank, client_time0);
        mpi_check(MPI_Recv(&utc, 1, MPI_LONG_INT, 0, 1, MPI_COMM_WORLD, &st));
        long int client_time1 = rand()%15;
        // correcting client time
        long int delta = round(((double)client_time1 + (double)client_time0) / 2);
        utc += delta;
        printf("Process: %d. T0: %lu, T1: %lu. Corrected time: %lu\n", rank, client_time0, client_time1, utc);
    }
    MPI_Finalize();
    return 0;
}