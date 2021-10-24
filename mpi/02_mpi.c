#include <mpi.h>
#include <stdio.h>

void mpi_check(int mpi_type) {
    if (mpi_type != MPI_SUCCESS) {
        printf("Erorr in MPI\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_type);
    }
}

int main(int argc, char *argv[]) {
    int size, rank, message = 0;
    MPI_Status st;

    mpi_check(MPI_Init(&argc, &argv));

    mpi_check(MPI_Comm_size(MPI_COMM_WORLD, &size));

    mpi_check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (rank == 0) {
        mpi_check(MPI_Send(&message, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD));
        printf("First rank. Message: %d. Send to %d from 0\n", message, rank + 1);

        mpi_check(MPI_Recv(&message, 1, MPI_INT, size - 1, 1, MPI_COMM_WORLD, &st));
        printf("Result: %d. From %d\n", message, size - 1);
    }
    else if (rank != size - 1) {
        mpi_check(MPI_Recv(&message, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &st));

        message += rank;

        mpi_check(MPI_Send(&message, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD));
        printf("Message: %d. Send to %d from %d\n", message, rank + 1, rank);
    }
    else {
        mpi_check(MPI_Recv(&message, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &st));

        message += rank;

        mpi_check(MPI_Send(&message, 1, MPI_INT, 0, 1, MPI_COMM_WORLD));
    }
    MPI_Finalize();
    return 0;
}
