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

int election(int rank, int size, int message, int timeout) {
    // send vote message to higher ranks
    for (int i = rank + 1; i < size; i++) {
        printf("Process %d. Sending vote message to %d...\n", rank, i);
        mpi_check(MPI_Send(&message, 1, MPI_INT, i, 2, MPI_COMM_WORLD));
    }

    (void) signal(SIGALRM, handler);
    alarm(timeout);
    MPI_Status st;
    // wait 'ok' message
    int ok = 0, flag = 0;

    while (sigflag) {
        mpi_check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &st));
        if (flag) {
            if (st.MPI_TAG == 2) {
                // check 'vote' message from lower ranks while wait answer message from higher ranks
                mpi_check(MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &st));
                printf("Process %d. Send 'ok' to %d\n", rank, st.MPI_SOURCE);
                mpi_check(MPI_Send(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD));
            }

            if (st.MPI_TAG == 1) {
                mpi_check(MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &st));
                printf("Process %d. Receive 'ok' from %d.\n", rank, st.MPI_SOURCE);
                alarm(0);
                // 'ok' message received, end while
                ok = 1;
                sigflag = 0;
            }
        }
    }
    return ok;
}

void order(int rank, int size, int message) {
    printf("New leader is %d.\n", rank);
    for (int i = 0; i < size; i++) {
        if (i == rank) {
            continue;
        }
        printf("Process %d. Sending message about new leader to %d...\n", rank, i);
        mpi_check(MPI_Send(&message, 1, MPI_INT, i, 3, MPI_COMM_WORLD));
    }
}

int main(int argc, char *argv[]) {
    int size, rank;
    MPI_Status st;
    mpi_check(MPI_Init(&argc, &argv));

    mpi_check(MPI_Comm_size(MPI_COMM_WORLD, &size));

    mpi_check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    srand(rank);

    // kill random process
    int lot = rand() % 2;
    if (rank != 1 && lot == 1) {
        printf("Process %d is dead\n", rank);
        MPI_Finalize();
        return 0;
    }

    int flag, ok, message = 0, timeout = 5;

    if (rank == 1) {
        // init first vote
        ok = election(rank, size, message, timeout);
        if (!ok) {
            order(rank, size, message);
        } else {
            mpi_check(MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &st));
            printf("Process %d. Receive message 'leader' from %d\n", rank, st.MPI_SOURCE);
        }
    } else {
        while (1) {
            mpi_check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &st));
            if (flag) {
                mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st));

                // tag = 2 is message "vote"
                if (st.MPI_TAG == 2) {
                    printf("Process %d. Send 'ok' to %d\n", rank, st.MPI_SOURCE);
                    // tag = 1 is message 'ok'
                    mpi_check(MPI_Send(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD));
                    ok = election(rank, size, message, timeout);

                    if (!ok) {
                        order(rank, size, message);
                        // new leader, end while
                        break;
                    }
                }

                // tag = 3 is message "leader"
                if (st.MPI_TAG == 3) {
                    printf("Process %d. Receive message 'leader' from %d\n", rank, st.MPI_SOURCE);
                    // new leader, end while
                    break;
                }
            }
        }
    }
    MPI_Finalize();
    return 0;
}