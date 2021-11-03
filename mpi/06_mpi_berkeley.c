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

    long int client_time;

    if (rank == 0) {
        // server time
        long int server_time = rand()%15;
        printf("Start server time: %li\n", server_time);

        // send server time to clients
        for (int i = 1; i < size; i++) {
            mpi_check(MPI_Send(&server_time, 1, MPI_LONG_INT, i, 1, MPI_COMM_WORLD));
        }

        // arrays for client ranks and diff time
        int ranks[size - 1];
        long int diffs[size - 1];
        double sum = 0;
        long int diff;

        // receive and save client diff time
        for (int i = 0; i < size - 1; i++) {
            mpi_check(MPI_Recv(&diff, 1, MPI_LONG_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &st));
            ranks[i] = st.MPI_SOURCE;
            diffs[i] = diff;
            sum += diff;
            printf("Receive from: %d. Diff time: %li\n", st.MPI_SOURCE, diff);
        }

        printf("Sum time: %f\n", sum);
        long int mean = lround(sum / size);
        printf("Mean time: %li\n", mean);
        long int adjustment;

        // send adjustment to client
        for (int i = 0; i < size - 1; i++) {
            adjustment = mean - diffs[i];
            printf("Send to %d, from 0, adjustment: %li\n", ranks[i], adjustment);
            mpi_check(MPI_Send(&adjustment, 1, MPI_LONG_INT, ranks[i], 2, MPI_COMM_WORLD));
        }

        printf("Rank 0. Corrected time: %li\n", server_time + mean);
    } else {
        long int server_time;
        // receive server time
        mpi_check(MPI_Recv(&server_time, 1, MPI_LONG_INT, 0, 1, MPI_COMM_WORLD, &st));

        client_time = rand()%15;
        printf("Rank: %d. Old time: %li\n", rank, client_time);
        long int diff = client_time - server_time;
        // send diff to server
        mpi_check(MPI_Send(&diff, 1, MPI_LONG_INT, 0, 1, MPI_COMM_WORLD));

        long int adjustment;

        mpi_check(MPI_Recv(&adjustment, 1, MPI_LONG_INT, 0, 2, MPI_COMM_WORLD, &st));

        // correct client time
        client_time += adjustment;
        printf("Rank: %d. Corrected time: %li\n", rank, client_time);

    }
    MPI_Finalize();
    return 0;
}