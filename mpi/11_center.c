#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void mpi_check(int mpi_type) {
    if (mpi_type != MPI_SUCCESS) {
        printf("Error in MPI\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_type);
    }
}

void critical_section(unsigned int time) {
    sleep(time);
}

void remainder_section(unsigned int time) {
    sleep(time);
}

int main(int argc, char *argv[]) {
    int size, rank;
    MPI_Status st;
    mpi_check(MPI_Init(&argc, &argv));

    mpi_check(MPI_Comm_size(MPI_COMM_WORLD, &size));

    mpi_check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    if (rank == 0) {
        printf("Leader is process %d\n", rank);
    }

    // init generator
    srand(rank);


    // tag 1 = request
    // tag 2 = permit
    // tag 3 = release
    int repeat = 3, flag = 0, message = 0, count = 0;
    if (rank == 0) {
        while (count < repeat * (size - 1)) {
            mpi_check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &st));
            if (flag) {
                if (st.MPI_TAG == 1) {
                    printf("0: Receive request from %d\n", st.MPI_SOURCE);
                    mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD, &st));

                    printf("0: Send permit to %d\n", st.MPI_SOURCE);
                    mpi_check(MPI_Send(&message, 1, MPI_INT, st.MPI_SOURCE, 2, MPI_COMM_WORLD));

                    printf("0: Receive release from %d\n", st.MPI_SOURCE);
                    mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, 3, MPI_COMM_WORLD, &st));
                    count += 1;
                }
            }
        }
    }
    else {
        while (count < repeat) {
            printf("%d: Send request to 0\n", rank);
            mpi_check(MPI_Send(&message, 1, MPI_INT, 0, 1, MPI_COMM_WORLD));

            printf("%d: Receive permit from %d\n", rank, st.MPI_SOURCE);
            mpi_check(MPI_Recv(&message, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &st));

            printf("%d: Start critical section\n", rank);
            critical_section(rand()%5 + 1);

            printf("%d: End critical section. Send release to 0\n", rank);
            mpi_check(MPI_Send(&message, 1, MPI_INT, 0, 3, MPI_COMM_WORLD));

            printf("%d: Start remainder section\n", rank);
            remainder_section(rand()%5 + 1);

            printf("%d: End remainder section\n", rank);
            count += 1;
        }
    }
    MPI_Finalize();
    return 0;
}
