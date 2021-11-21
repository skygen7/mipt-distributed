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

int *election(int rank, int size, int *processes, int timeout) {
    MPI_Status st;
    (void) signal(SIGALRM, handler);
    int next = (rank + 1) % size;
    int flag = 0, message = 0, ok = 0;
    int next_elem = rank;
    while (!ok) {
        if (next == rank) {
            printf("Other processes offline. Process %d is the leader\n", rank);
            break;
        }
        sigflag = 1;
        printf("Process %d. Send message 'vote' to %d\n", rank, next);
        mpi_check(MPI_Send(processes, size, MPI_INT, next, 2, MPI_COMM_WORLD));
        alarm(timeout);
        while (sigflag) {
            mpi_check(MPI_Iprobe(next, 1, MPI_COMM_WORLD, &flag, &st));
            if (flag) {
                mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD, &st));
                alarm(0);
                printf("Process %d. Receive message 'ok' from %d\n", rank, st.MPI_SOURCE);
                sigflag = 0;
                ok = 1;
                next_elem = next;
            }
        }
        next = (next + 1) % size;
    }

    int *response = malloc(2 * sizeof(int));
    response[0] = ok;
    response[1] = next_elem;
    return response;
}

int find_winner(int size, int *processes) {
    int max = processes[0];
    for (int i = 0; i < size; i++) {
        max = processes[i] > max ? processes[i] : max;
    }
    return max;
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
        processes[i] = -1;
    }

    int ok, next_elem, timeout = 2, flag = 0, message = 0;
    int *response = malloc(2 * sizeof(int));

    if (rank == 1) {
        processes[rank] = rank;
        printf("Process %d. Add myself to array\n", rank);

        response = election(rank, size, processes, timeout);
        ok = response[0];

        if (ok) {
            while (1) {
                mpi_check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &st));

                if (flag) {
                    if (st.MPI_TAG == 2) {
                        mpi_check(MPI_Recv(processes, size, MPI_INT, st.MPI_SOURCE, 2, MPI_COMM_WORLD, &st));
                        printf("Process %d. Receive message 'vote' from %d\n", rank, st.MPI_SOURCE);

                        mpi_check(MPI_Send(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD));
                        printf("Process %d. Send 'ok' message to %d\n", rank, st.MPI_SOURCE);

                        if (processes[rank] == rank) {
                            printf("Process %d. Stop election, calculate max rank...\n", rank);
                            int max = find_winner(size, processes);
                            printf("Process %d. New leader is %d\n", rank, max);

                            next_elem = response[1];
                            mpi_check(MPI_Send(&message, 1, MPI_INT, next_elem, 3, MPI_COMM_WORLD));
                            printf("Process %d. Send message about new leader to %d\n", rank, next_elem);
                            break;
                        }
                    }
                    if (st.MPI_TAG == 3) {
                        mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, 3, MPI_COMM_WORLD, &st));
                        printf("Process %d. Receive message about leader from %d\n", rank, st.MPI_SOURCE);
                        break;
                    }
                }
            }
        }
    } else {
        while (1) {
            mpi_check(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &st));

            if (flag) {
                if (st.MPI_TAG == 2) {
                    mpi_check(MPI_Recv(processes, size, MPI_INT, st.MPI_SOURCE, 2, MPI_COMM_WORLD, &st));
                    printf("Process %d. Receive message 'vote' from %d\n", rank, st.MPI_SOURCE);

                    mpi_check(MPI_Send(&message, 1, MPI_INT, st.MPI_SOURCE, 1, MPI_COMM_WORLD));
                    printf("Process %d. Send 'ok' message to %d\n", rank, st.MPI_SOURCE);

                    processes[rank] = rank;
                    printf("Process %d. Add myself to array\n", rank);

                    response = election(rank, size, processes, timeout);
                    next_elem = response[1];
                }
                if (st.MPI_TAG == 3) {
                    mpi_check(MPI_Recv(&message, 1, MPI_INT, st.MPI_SOURCE, 3, MPI_COMM_WORLD, &st));
                    printf("Process %d. Receive message about leader from %d\n", rank, st.MPI_SOURCE);

                    mpi_check(MPI_Send(&message, 1, MPI_INT, next_elem, 3, MPI_COMM_WORLD));
                    printf("Process %d. Send message about new leader to %d\n", rank, next_elem);
                    break;
                }
            }
        }
    }
    MPI_Finalize();
    return 0;
}
