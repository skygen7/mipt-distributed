#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int sigflag = 1;
void handler(int nsig) {
    sigflag = 0;
}

void mpi_check(int mpi_type) {
    if (mpi_type != MPI_SUCCESS) {
        printf("Error in MPI\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_type);
    }
}

void critical_section(unsigned int time) {
    sleep(time);
}

void remainder_section(unsigned int timeout, int rank, int next, int prev, int token) {
    MPI_Status st;
    (void) signal(SIGALRM, handler);
    int flag = 0;

    alarm(timeout);
    while (sigflag) {
        mpi_check(MPI_Iprobe(prev, 1, MPI_COMM_WORLD, &flag, &st));
        if (flag) {
            if (st.MPI_TAG == 1) {
                mpi_check(MPI_Recv(&token, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD, &st));

                mpi_check(MPI_Send(&token, 1, MPI_INT, next, 1, MPI_COMM_WORLD));
            }
        }
    }
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
    int critical_time = rand() % 2 + 1;
    int timeout = rand() % 5 + 1;
    int repeat = 3, count = 0, message = 0;
    int next = (rank + 1) % size;
    int prev = rank - 1;

    // tag 1 = token
    // tag 2 = FIN

    if (rank == 0) {
        mpi_check(MPI_Send(&message, 1, MPI_INT, next, 1, MPI_COMM_WORLD));
        prev = size - 1;
    }

    while (count < repeat) {
        mpi_check(MPI_Recv(&message, 1, MPI_INT, prev, 1, MPI_COMM_WORLD, &st));

        printf("%d: Start critical section\n", rank);
        critical_section(critical_time);
        printf("%d: End critical section\n", rank);

        mpi_check(MPI_Send(&message, 1, MPI_INT, next, 1, MPI_COMM_WORLD));

        printf("%d: Start remainder section\n", rank);
        remainder_section(timeout, rank, next, prev, message);
        printf("%d: End remainder section\n", rank);
        count += 1;

    }

    if (rank == 0) {
        printf("%d: Send 'FIN' to %d\n", rank, next);
        mpi_check(MPI_Send(&message, 1, MPI_INT, next, 2, MPI_COMM_WORLD));
    }

    count = 0;
    while (count < 2) {
        mpi_check(MPI_Recv(&message, 1, MPI_INT, prev, MPI_ANY_TAG, MPI_COMM_WORLD, &st));
        if (st.MPI_TAG == 1) {
            mpi_check(MPI_Send(&message, 1, MPI_INT, next, 1, MPI_COMM_WORLD));
        }
        if (st.MPI_TAG == 2) {
            printf("%d: Receive 'FIN' from %d\n", rank, st.MPI_SOURCE);
            mpi_check(MPI_Send(&message, 1, MPI_INT, next, 2, MPI_COMM_WORLD));
            printf("%d: Send 'FIN' to %d\n", rank, next);
            count += 1;
        }

        if (count == 2) {
            printf("%d: Receive 'FIN' twice. Disconnect\n", rank);
        }
    }

    MPI_Finalize();
    return 0;
}
