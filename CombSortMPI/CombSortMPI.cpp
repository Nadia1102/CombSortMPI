#include "pch.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <mpi.h>
using namespace std;
void swap(int *arr, int i, int j)
{
	int t = arr[i];
	arr[i] = arr[j];
	arr[j] = t;
}

void combSort(int *arr, int size)
{
	double fakt = 1.2473309;

	int gap = size;
	bool swapped = true;

	while (gap != 1 || swapped == true)
	{
		gap /= fakt;

		if (gap < 1) {
			gap = 1;
		}

		swapped = false;

		for (int i = 0; i < size - gap; i++)
		{
			if (arr[i] > arr[i + gap])
			{
				swap(arr, i, i + gap);
				swapped = true;
			}
		}
	}
}


int * merge(int * arr1, int size1, int * arr2, int size2)
{
	int sum = size1 + size2;
	int * result = (int *)malloc(sum * sizeof(int));

	int i = 0;
	int j = 0;
	for (int k = 0; k < sum; k++) {
		if (i >= size1) {
			result[k] = arr2[j];
			j++;
		}
		else if (j >= size2) {
			result[k] = arr1[i];
			i++;
		}
		else if (arr1[i] < arr2[j]) {
			result[k] = arr1[i];
			i++;
		}
		else {
			result[k] = arr2[j];
			j++;
		}
	}

	return result;
}

int main(int argc, char ** argv)
{
	int procCount, rank;
	double startTime;

	int size = 1000000;
	int * arr = new int[size];
	int * chunk;
	int maxChunkSize, chunkSize;
	int * receivedChunk;
	int receivedChunkSize;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	ifstream input;
	input.open("input1m.txt");

	for (int i = 0; i < size; i++) {
		input >> arr[i];
	}
		 
	MPI_Barrier(MPI_COMM_WORLD);
	startTime = MPI_Wtime();

	MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	maxChunkSize = size / procCount;
	if (size % procCount != 0) {
		maxChunkSize++;
	}

	chunk = (int *)malloc(maxChunkSize * sizeof(int));
	MPI_Scatter(arr, maxChunkSize, MPI_INT, chunk, maxChunkSize, MPI_INT, 0, MPI_COMM_WORLD);
	free(arr);
	arr = NULL;

	chunkSize = (size >= maxChunkSize * (rank + 1)) ? maxChunkSize : size - maxChunkSize * rank;
	combSort(chunk, chunkSize);

	for (int step = 1; step < procCount; step = 2 * step) {
		if (rank % (2 * step) != 0) {
			MPI_Send(chunk, chunkSize, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
			break;
		}

		if (rank + step < procCount) {
			receivedChunkSize = size >= maxChunkSize * (rank + 2 * step)
				? maxChunkSize * step 
				: size - maxChunkSize * (rank + step);

			receivedChunk = (int *)malloc(receivedChunkSize * sizeof(int));
			MPI_Recv(receivedChunk, receivedChunkSize, MPI_INT, rank + step, 0, MPI_COMM_WORLD, &status);

			arr = merge(chunk, chunkSize, receivedChunk, receivedChunkSize);

			free(chunk);
			free(receivedChunk);

			chunk = arr;
			chunkSize += receivedChunkSize;
		}
	}

	double endTime = MPI_Wtime();
	std::cout << "Time: " << endTime - startTime << std::endl;
	//ofstream out;
	//out.open("output.txt");

	//if (out.is_open())
	//{
	//	out << "Time: " << endTime - startTime << std::endl;
	//	/* for (int i = 0; i < chunkSize; i++) {
	//		out << chunk[i] << " ";
	//	} */
	//}

	MPI_Finalize();
}