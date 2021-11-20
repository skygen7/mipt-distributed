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

int election(int rank, int size, int *processes, int timeout) {
    MPI_Status st;
    (void) signal(SIGALRM, handler);
    int next = (next + 1) % rank;
    int flag = 0, message = 0, ok = 0;
    while (!ok) {
        if (next == rank) {
            printf("Other processes offline. Process %d is the leader\n", rank);
            break;
        }
        printf("Process %d. Send message 'vote' to %d\n", rank, next);
        mpi_check(MPI_Send(processes, size, MPI_INT, next, 2, MPI_COMM_WORLD));
        alarm(timeout);
        while (sigflag) {
            mpi_check(MPI_Iprobe(next, 1, MPI_COMM_WORLD, &flag, &st));
            if (flag) {
                mpi_check(MPI_Recv(&message, 1, MPI_INT, next, 1, MPI_COMM_WORLD, &st));
                printf("Process %d. Receive message 'ok' from %d\n", rank, st.MPI_SOURCE);
                sigflag = 0;
                ok = 1;
            }

        }
        next = (next + 1) % rank;
    }
    return ok;
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

    int *processes = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        processes[i] = 0;
    }
    int ok, count = 0, timeout = 2, flag = 0, message = 0;
    if (rank == 1) {
        processes[rank] = 1;
        ok = election(rank, size, processes, timeout);
        if (ok) {
            while (1) {
                mpi_check(MPI_Iprobe(MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &flag, &st));
                if (flag) {
                    mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, 3, MPI_COMM_WORLD, &st));
                    printf("Process %d. Receive message 'leader' from %d\n", rank, st.MPI_SOURCE);
                    break;
                }
            }
        }
    }

}
