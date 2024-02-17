#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

struct complex {
    double real;
    double imag;
};

int cal_pixel(struct complex c) {
    double z_real = 0;
    double z_imag = 0;
    int iter = 0;
    while (z_real * z_real + z_imag * z_imag < 4.0 && iter < MAX_ITER) {
        double z_real_temp = z_real * z_real - z_imag * z_imag + c.real;
        z_imag = 2 * z_real * z_imag + c.imag;
        z_real = z_real_temp;
        iter++;
    }
    return iter;
}
void save_pgm(const char *filename, int height, int width, int image[height][width]) {
    FILE *pgmimg;
    pgmimg = fopen(filename, "wb");
    fprintf(pgmimg, "P2\n");
    fprintf(pgmimg, "%d %d\n", width, height);
    fprintf(pgmimg, "255\n");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(pgmimg, "%d ", image[i][j]);
        }
        fprintf(pgmimg, "\n");
    }
    fclose(pgmimg);
}

int main() {
    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows = HEIGHT / size;
    int rem = HEIGHT % size;
    if (rank < rem) {
        rows++;
    }
    int local_image[rows][WIDTH];
    for (int i = rank * rows; i < (rank + 1) * rows; i++) {
        for (int j = 0; j < WIDTH; j++) {
            struct complex c = {.real = (j - WIDTH / 2.0) * 4.0 / WIDTH, .imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT};
            local_image[i - rank * rows][j] = cal_pixel(c);
      }
      
    }
    int global_image[HEIGHT][WIDTH];
    if (rank == 0) {
        MPI_Gather(local_image, rows * WIDTH, MPI_INT, global_image, rows * WIDTH, MPI_INT, 0, MPI_COMM_WORLD);
        save_pgm("mandelbrot.pgm", HEIGHT, WIDTH, global_image);
    } else {
        MPI_Gather(local_image, rows * WIDTH, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}