#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    const int MAX_NUMBERS = 2000;
    int numbers[MAX_NUMBERS];
    int result_sum = 0;
    int random_amount;
    int number_random;

    // Define M as 5 times the number of processes
    const int M = 5 * world_size;

    if (world_rank == 0)
    {
        srand(time(NULL));
        int values_per_process = M / (world_size - 1);

        // Track which slaves have finished processing
        int completed_processes = 0;
        int completed[world_size - 1];
        for (int i = 0; i < world_size - 1; i++)
        {
            completed[i] = 0;
        }

        for (int j = 1; j < world_size; j++)
        {
            // Generate values for each process
            random_amount = values_per_process;
            for (int i = 0; i < random_amount; i++)
            {
                number_random = rand() % 100;  // Numbers between 0 and 99
                numbers[i] = number_random;
            }

            // Send the number of values and the values themselves
            MPI_Send(&random_amount, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
            MPI_Send(numbers, random_amount, MPI_INT, j, 0, MPI_COMM_WORLD);
        }

        // Receive results from slaves and keep track of the total sum
        while (completed_processes < world_size - 1)
        {
            int finished_rank;
            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            finished_rank = status.MPI_SOURCE;

            int value;
            MPI_Recv(&value, 1, MPI_INT, finished_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            result_sum += value;
            completed[finished_rank - 1] = 1; // Mark process as completed
            completed_processes++;
            printf("Received %d from process %d\n", value, finished_rank);
        }

        printf("Total sum from slave processes: %d\n", result_sum);
    }
    else
    {
        // Slaves receive values and calculate their sum
        MPI_Recv(&random_amount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(numbers, random_amount, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Calculate the sum of the received values
        for (int i = 0; i < random_amount; i++)
        {
            result_sum += numbers[i];
        }

        // Send the result sum back to the master
        MPI_Send(&result_sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        printf("Process %d sent the sum %d to Master\n", world_rank, result_sum);
    }

    MPI_Finalize();
    return 0;
}
