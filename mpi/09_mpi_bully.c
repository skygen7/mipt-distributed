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

    int flag;
    int timeout = 5;
    int message = 0;
    int leader = 1;

    (void) signal(SIGALRM, handler);
    alarm(timeout);
    while (sigflag) {
        if (rank == leader) {
            for (int i = rank + 1; i < size; i++) {
                mpi_check(MPI_Send(&message, 1, MPI_INT, i, 2, MPI_COMM_WORLD));
            }
        }
        mpi_check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &st));
        if (flag) {
            mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD, &st));
            // tag = 1 is message "ok"
            if (st.MPI_TAG == 1) {
                timeout = 0;
            }

            // tag = 2 is message "vote"
            if (st.MPI_TAG == 2) {
                mpi_check(MPI_Send(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD));
            }

            // tag = 3 is message "leader"
            if (st.MPI_TAG == 3) {
                mpi_check(MPI_Send(&message, 1, MPI_INT, st.MPI_SOURCE, 3, MPI_COMM_WORLD));
            }
        }
    }
    if (!flag) {

    }

}

