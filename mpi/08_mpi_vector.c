#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


void mpi_check(int mpi_type) {
    if (mpi_type != MPI_SUCCESS) {
        printf("Error in MPI\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_type);
    }
}

char *vec_to_str(char *svector, int *vector, int size) {

    for (int i = 0; i < size; i++) {
        svector[i] = vector[i] + '0';
    }
    svector[size] = '\0';
    return svector;
}

int main(int argc, char *argv[]) {
    int size, rank;
    MPI_Status st;
    mpi_check(MPI_Init(&argc, &argv));

    mpi_check(MPI_Comm_size(MPI_COMM_WORLD, &size));

    mpi_check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    // create dynamic filename
    char filename[BUFSIZ];
    sprintf(filename, "v%02d.dat", rank + 1);
    FILE *fp;

    if ((fp=fopen(filename, "r")) == NULL) {
        printf("File '%s' not found for rank: %d\n", filename, rank);
        MPI_Finalize();
        return 0;
    }

    // create empty vector
    int *vector = malloc(size * sizeof(int));
    int k = 0, time = 0;
    // create string vector for print
    char *svector = malloc(size * sizeof(char));

    for (int i = 0; i < size; i++) {
        vector[i] = 0;
    }

    while (!feof(fp)) {
        fscanf(fp, "%d", &k);
        printf("Rank %d. File: %s\n", rank, filename);

        if (k == 0) {
            time += 1;
            vector[rank] = time;
            svector = vec_to_str(svector, vector, size);
            printf("Rank: %d. Internal state: %d. Vector time: %s\n", rank, k, svector);
        } else if (k > 0) {
            time += 1;
            vector[rank] = time;

            mpi_check(MPI_Send(vector, size, MPI_INT, k - 1, 0, MPI_COMM_WORLD));

            svector = vec_to_str(svector, vector, size);
            printf("Send to: %d from %d. Vector time: %s\n", k - 1, rank, svector);
        } else {
            mpi_check(MPI_Recv(vector, size, MPI_INT, abs(k) - 1, 0, MPI_COMM_WORLD, &st));

            time += 1;
            int message = vector[rank];
            time = message > time ? message : time;
            vector[rank] = time;

            svector = vec_to_str(svector, vector, size);
            printf("Rank: %d. Receive from %d. Corrected: %s\n", rank, abs(k) - 1, svector);
        }
    }
    fclose(fp);
    free(vector);
    free(svector);
    MPI_Finalize();
    return 0;
}

