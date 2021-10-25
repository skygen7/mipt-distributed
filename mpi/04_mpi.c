#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void mpi_check(int mpi_type) {
    if (mpi_type != MPI_SUCCESS) {
        printf("Erorr in MPI\n");
        MPI_Abort(MPI_COMM_WORLD, mpi_type);
    }
}

double square(int N, int rankbeg, int rankend) {
    double h, x, next_x, square, result = 0.0;
    h = 2.0 / N;
    for (int i = rankbeg; i <= rankend; i++) {
        x = h * i;
        next_x = h * (i + 1);
        square = (h * (sqrt(4 - pow(x, 2)) + sqrt(4 - pow(next_x, 2)))) / 2;
        result += square;
    }
    return result;
}

int main(int argc, char *argv[]) {
    int size, rank;
    MPI_Status st;
    mpi_check(MPI_Init(&argc, &argv));

    mpi_check(MPI_Comm_size(MPI_COMM_WORLD, &size));

    mpi_check(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    mpi_check(MPI_Barrier(MPI_COMM_WORLD));

    if (rank == 0) {
        // Считываем N из файла
        const char filename[BUFSIZ] = "N.dat";
        FILE *fp;
        int N = 0;
        if ((fp=fopen(filename, "r")) == NULL) {
            printf("Can't open file '%s'\n", filename);
            exit(1);
        }
        fscanf(fp, "%d", &N);
        fclose(fp);

        // Отправляем N остальным процессам
        mpi_check(MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD));

        // Считаем сумму площадей трапеций нулевого
        double result = 0.0;
        int rankbeg = 0;
        int rankend = (rank + 1) * N / size - 1;

        // Принимаем сумму площадей трапеций от остальных
        double sumrank;
        mpi_check(MPI_Reduce(&sumrank, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD));
        // Прибавляем нулевую площадь
        result += square(N, rankbeg, rankend);
        // Окончательный результат
        printf("Result: %f\n", result);
    } else {
        // прием N от нулевого
        int N;

        mpi_check(MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD));

        int rankbeg = rank * N / size;
        int rankend;

        if (rank < size - 1) {
            rankend = (rank + 1) * N / size - 1;
        } else {
            rankend = N - 1;
        }
        printf("Process: %d. Rankbeg: %d, rankend: %d\n", rank, rankbeg, rankend);

        // вычисляем сумму трапеций от rankbeg до rankend и отправляем 0
        double sumrank = square(N, rankbeg, rankend);
        mpi_check(MPI_Reduce(&sumrank, &sumrank, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD));
        printf("Sumrank: %f. Send to 0 from %d\n", sumrank, rank);
    }

    MPI_Finalize();
    return 0;
}