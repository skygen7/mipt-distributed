#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void mpi_check(int mpi_type) {
    if (mpi_type != MPI_SUCCESS) {
        printf("Error in MPI\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_type);
    }
}


int main(int argc, char *argv[]) {
    int size, rank;
    MPI_Status st;
    mpi_check(MPI_Init(&argc, &argv));

    mpi_check(MPI_Comm_size(MPI_COMM_WORLD, &size));

    mpi_check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    // start random generator value
    srand(rank + 1);

    if (rank == 0) {
        // server time
        long int utc = rand()%15;
        printf("Start server time: %lu\n", utc);
        int proc;
        for (int i = 1; i < size; i++) {
            mpi_check(MPI_Recv(&proc, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &st));

            // send utc to client
            mpi_check(MPI_Send(&utc, 1, MPI_LONG_INT, proc, 1, MPI_COMM_WORLD));
            printf("Send to %d UTC: %lu\n", proc, utc);
            utc += rand()%15;
        }
    } else {
        // send current rank to server
        mpi_check(MPI_Send(&rank, 1, MPI_INT, 0, 1, MPI_COMM_WORLD));

        // get server time
        long int client_time0 = rand()%15;
        long int utc;
        mpi_check(MPI_Recv(&utc, 1, MPI_LONG_INT, 0, 1, MPI_COMM_WORLD, &st));

        long int client_time1 = client_time0 + rand()%15;
        printf("Process: %d. Client time - T0: %lu, T1: %lu. UTC: %lu\n", rank, client_time0, client_time1, utc);
        // correcting client time
        long int delta = round(((double)client_time1 - (double)client_time0) / 2);
        if (client_time1 != utc + delta) {
              client_time1 = utc + delta;
              printf("Process: %d. New Client time - T0: %lu, T1 (corrected): %lu. UTC: %lu\n", rank, client_time0, client_time1, utc);
        }
    }
    MPI_Finalize();
    return 0;
}