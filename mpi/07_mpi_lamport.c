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

    char filename[BUFSIZ];
    sprintf(filename, "%02d.dat", rank + 1);
    FILE *fp;
    int k = 0;
    if ((fp=fopen(filename, "r")) == NULL) {
        printf("File '%s' not found for rank: %d\n", filename, rank);
        MPI_Finalize();
        return 0;
    }

    int time = 0;
    int message;

    while(!feof(fp)) {
        fscanf(fp, "%d", &k);
        printf("Rank %d. File: %s\n", rank, filename);
        if (k == 0) {
            time += 1;
            printf("Rank: %d. Internal state: %d. Time: %d\n", rank, k, time);
        } else if (k > 0) {
            time += 1;
            mpi_check(MPI_Send(&time, 1, MPI_INT, k - 1, 0, MPI_COMM_WORLD));
            printf("Send to: %d from %d. Time: %d\n", k - 1, rank, time);
        } else {
            mpi_check(MPI_Recv(&message, 1, MPI_INT, abs(k) - 1, 0, MPI_COMM_WORLD, &st));
            time += 1;
            message += 1;
            time = message > time ? message : time;
            printf("Rank: %d. Receive from %d. Corrected: %d\n", rank, abs(k) - 1, time);
        }
    }
    fclose(fp);
    MPI_Finalize();
    return 0;
}
