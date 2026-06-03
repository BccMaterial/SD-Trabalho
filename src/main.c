#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank, size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int max_row, max_column, max_n;

  if (rank == 0) {
    if (argc != 4) {
      fprintf(stderr, "Uso: %s linhas colunas iteracoes\n", argv[0]);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    max_row = atoi(argv[1]);
    max_column = atoi(argv[2]);
    max_n = atoi(argv[3]);
  }

  MPI_Bcast(&max_row, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&max_column, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&max_n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int base_rows = max_row / size;
  int remainder = max_row % size;

  int local_rows = base_rows + (rank < remainder ? 1 : 0);

  int start_row = rank * base_rows + (rank < remainder ? rank : remainder);

  char *local = malloc(local_rows * max_column);

  for (int r = 0; r < local_rows; r++) {
    int global_r = start_row + r;

    for (int c = 0; c < max_column; c++) {
      float zr = 0.0f;
      float zi = 0.0f;

      float cr = (float)c * 2.0f / max_column - 1.5f;

      float ci = (float)global_r * 2.0f / max_row - 1.0f;

      int n = 0;

      while ((zr * zr + zi * zi) < 4.0f && ++n < max_n) {
        float new_zr = zr * zr - zi * zi + cr;

        float new_zi = 2.0f * zr * zi + ci;

        zr = new_zr;
        zi = new_zi;
      }

      local[r * max_column + c] = (n == max_n) ? '#' : '.';
    }
  }

  char *global = NULL;
  int *recvcounts = NULL;
  int *displs = NULL;

  if (rank == 0) {
    global = malloc(max_row * max_column);

    recvcounts = malloc(size * sizeof(int));

    displs = malloc(size * sizeof(int));

    int offset = 0;

    for (int p = 0; p < size; p++) {
      int rows = base_rows + (p < remainder ? 1 : 0);

      recvcounts[p] = rows * max_column;

      displs[p] = offset;

      offset += recvcounts[p];
    }
  }

  MPI_Gatherv(local, local_rows * max_column, MPI_CHAR, global, recvcounts,
              displs, MPI_CHAR, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (int r = 0; r < max_row; r++) {
      for (int c = 0; c < max_column; c++) {
        putchar(global[r * max_column + c]);
      }

      putchar('\n');
    }

    free(global);
    free(recvcounts);
    free(displs);
  }

  free(local);

  MPI_Finalize();

  return 0;
}
