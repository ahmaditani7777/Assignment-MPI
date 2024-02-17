#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255
struct complex{
    double real;
    double imag;};

int calculate_pixel(struct complex c){
    double z_real = c.real;
    double z_imag = c.imag;
    int n = 0;
    
    while((z_real* z_real +z_imag* z_imag <=4.0) && (n< MAX_ITER)) {
        double z_real_temp = z_real * z_real - z_imag * z_imag + c.real;
        z_imag = 2 * z_real * z_imag + c.imag;
        z_real = z_real_temp;
        n++;
    }
    return n;
}

void master_process(int world_size){
    int rows[HEIGHT];
    for (int i = 0; i < HEIGHT; i++) {
        rows[i] = 0;
    }
    int num_rows_processed = 0;
    MPI_Status status;
    int result[WIDTH]; 
    int row_index;

    for(int i = 1; i < world_size; i++){
        row_index = i - 1; 
        MPI_Send(&row_index, 1, MPI_INT, i, 0, MPI_COMM_WORLD) ;
        rows[row_index] = 1; 
    }
    while(num_rows_processed < HEIGHT ){
        MPI_Recv(result, WIDTH, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        num_rows_processed++;
        for (row_index = 0; row_index < HEIGHT; row_index++) {
            if (rows[row_index] == 0) {
                MPI_Send(&row_index, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                rows[row_index] = 1;
                break;
            }
      }
    }
    row_index = -1;
    for (int i = 1;i<world_size;i++){
        MPI_Send(&row_index, 1, MPI_INT, i, 0, MPI_COMM_WORLD );
     } }

void worker_process(){
    MPI_Status status;
    int row_index;
    struct complex c;
    int result[WIDTH];
    while (1){
        MPI_Recv(&row_index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        if(row_index == -1 ) break; 

        for (int j = 0; j < WIDTH; j++){
            c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH ;
            c.imag = (row_index - HEIGHT / 2.0) * 4.0 / HEIGHT;
            result[j]= calculate_pixel(c);
        }
        MPI_Send(result, WIDTH, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }}
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size < 2) {
        fprintf(stderr, "Requires at least two processes.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (world_rank == 0){
        master_process(world_size);
    }else
    {worker_process();}

    MPI_Finalize();
}