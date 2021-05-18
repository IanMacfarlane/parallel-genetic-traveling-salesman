#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

int distance[100][100];// returns distance between city a and b

int pathDistance(int path[100]) {

	int dist = 0;
	for (int i = 0; i < 99; i++) {
		dist += distance[path[i]][path[i+1]];
	}
	dist += distance[path[0]][path[99]];

	return dist;
}

int main(int argc, char ** argv)
{
	int rank, size, data[1], flag;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Request request;
	MPI_Status status;

	srand(time(NULL));

	// read in city locations from cities.txt
	int locationX[100];// returns x location of city a
	int locationY[100];// returns y location of city a
	FILE *fp;
	fp = fopen("cities.txt", "r");
	char buff[6];
	for (int i = 0; i < 200; i++) {
		fscanf(fp, "%s", buff);
		if (i%2 == 0) {
			sscanf(buff, "%d", &locationX[i/2]);
		}
		else {
			sscanf(buff, "%d", &locationY[(i-1)/2]);
		}
	}

	// fill 2d array with distance between each pair of cities
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			// calculate distance between two cities using distance formula
			distance[i][j] = sqrt(pow(locationX[i]-locationX[j], 2) + pow(locationY[i]-locationY[j], 2));
		}
	}

	if (rank == 0) {

		// initialize global population with random paths
		int population[1000][100];// array of paths
		for (int i = 0; i < 1000; i++) {
			// fill path in population
			for (int j = 0; j < 100; j++) {
				population[i][j] = j;
			}

			// randomize path
			for (int j = 0; j < 1000; j++) {
				int a = rand()%100;
				int b = rand()%100;
				population[i][a] = b;
				population[i][b] = a;
			}
		}

		int shortest = INT_MAX;
		int iterations = 0;
		while (iterations < 100000) {

			// TODO distribute sub populations

			// calculate fitness for each sub population
			int fitness[1000];// contains path distance for each path in population
			int order[1000];// returns location of path in population
			for (int i = 0; i < 1000; i++) {
				fitness[i] = pathDistance(population[i]);
				order[i] = i;
			}

			// perform selection by sorting fitness from lowest to greatest, each even path is paired with the following odd path
			int sorted = 0;
			while (sorted == 0) {
				sorted = 1;
				for (int i = 0; i < 999; i++) {
					if (fitness[i] > fitness[i+1]) {
						sorted = 0;
						int temp = fitness[i];
						fitness[i] = fitness[i+1];
						fitness[i+1] = temp;

						temp = order[i];
						order[i] = order[i+1];
						order[i+1] = temp;
					}
				}
			}

			// print shortest path of iteration
			if (shortest > fitness[0]) {
				shortest = fitness[0];
				printf("Iteration %d: %d\n", iterations, shortest);// TODO output run time
			}

			// perform crossover
			for (int i = 0; i < 500; i+=2) {
				// select random crossover point
				int point = rand()%99;

				for (int j = 0; j < 100; j++) {
					if (j <= point) {
						population[order[i+500]][j] = population[order[i]][j];
						population[order[i+501]][j] = population[order[i+1]][j];
					}
					else {
						population[order[i+500]][j] = population[order[i+1]][j];
						population[order[i+501]][j] = population[order[i]][j];
					}
				}
			}

			// TODO repopulate global population

			// perform random mutation
			for (int i = 0; i < 1000; i++) {
				// randomly swap two cities
				int a = rand()%100;
				int b = rand()%100;
				population[i][a] = b;
				population[i][b] = a;
			}

			iterations++;
		}

		printf("Shortest Path: %d\n", shortest);
	}

	MPI_Finalize();
	return 0;
}
