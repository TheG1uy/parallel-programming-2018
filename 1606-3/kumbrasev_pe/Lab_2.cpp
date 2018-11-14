#include "mpi.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <cmath>
#define root 0

int* compare_mas(int *vec1, int* vec2, int len) {
	int *tempBuf = (int*)malloc(sizeof *vec1 * len * 2);
	int i = 0, j = 0, k = 0;
	while (i < len && j < len)
		vec1[i] < vec2[j] ? tempBuf[k++] = vec1[i++] : tempBuf[k++] = vec2[j++];
	while (i < len) tempBuf[k++] = vec1[i++];
	while (j < len) tempBuf[k++] = vec2[j++];
	return tempBuf;
}

int main(int argv, char *argc[]) {


	int size = 200000;
	int tempSize;
	int *mas = (int*)malloc(sizeof(int) * size);
	int *buf = nullptr;
	int *sub = nullptr;
	int ProcNum;
	int ProcRank;
	double start, end;
	MPI_Status status;
	srand(time(0));
	for (int i = 0; i < size; i++)
		mas[i] = rand() % 30;
	MPI_Init(&argv, &argc);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	if (!ProcRank) {
		(size % ProcNum) ? tempSize = (size + ProcNum - 1) / ProcNum * ProcNum : tempSize = size;
		buf = (int*)malloc(sizeof(int) * tempSize);
		memcpy(buf + (tempSize - size), mas, size * sizeof *mas);
		for (int i = 0; i < tempSize - size; i++) 
			buf[i] = -1;
		start = MPI_Wtime();

	}

	MPI_Bcast(&tempSize, 1, MPI_INT, root, MPI_COMM_WORLD);
	int *local_vec = (int*)malloc(sizeof(int) * (tempSize / ProcNum));
	sub = (int*)malloc(sizeof(int) * (tempSize / ProcNum));
	MPI_Scatter(buf, tempSize / ProcNum , MPI_INT, local_vec, tempSize / ProcNum, MPI_INT, root, MPI_COMM_WORLD);

	qsort(local_vec, tempSize / ProcNum, sizeof *local_vec, [](const void *a, const void *b) {
		int arg1 = *reinterpret_cast<const int*>(a);
		int arg2 = *reinterpret_cast<const int*>(b);
		if (arg1 < arg2)  return -1;
		if (arg1 > arg2)  return 1;
		return 0;
	});


	for (int i = 1; i < ProcNum + 1; i++) {
		if (i % 2 != 0) {
			if (ProcRank % 2) {
				if (ProcRank < ProcNum - 1) {
					MPI_Sendrecv(local_vec, tempSize / ProcNum, MPI_INT, ProcRank + 1,
						0, sub, tempSize / ProcNum, MPI_INT, ProcRank + 1, 0, MPI_COMM_WORLD, &status);
					sub = compare_mas(local_vec, sub, tempSize / ProcNum);
					memcpy(local_vec, sub, tempSize / ProcNum * sizeof *sub);
				}

			}
			else if (ProcRank > 0) {
				MPI_Sendrecv(local_vec, tempSize / ProcNum, MPI_INT, ProcRank - 1,
					0, sub, tempSize / ProcNum, MPI_INT, ProcRank - 1, 0, MPI_COMM_WORLD, &status);
				sub = compare_mas(local_vec, sub, tempSize / ProcNum);
				memcpy(local_vec, sub + tempSize / ProcNum, tempSize / ProcNum * sizeof *sub);
			}
		}
		else {
			if (!(ProcRank % 2)) {
				if (ProcRank < ProcNum - 1) {
					MPI_Sendrecv(local_vec, tempSize / ProcNum, MPI_INT, ProcRank + 1,
						0, sub, tempSize / ProcNum, MPI_INT, ProcRank + 1, 0, MPI_COMM_WORLD, &status);
					sub = compare_mas(local_vec, sub, tempSize / ProcNum);
					memcpy(local_vec, sub, tempSize / ProcNum * sizeof *sub);
				}

			}
			else {
				MPI_Sendrecv(local_vec, tempSize / ProcNum, MPI_INT, ProcRank - 1,
					0, sub, tempSize / ProcNum, MPI_INT, ProcRank - 1, 0, MPI_COMM_WORLD, &status);
				sub = compare_mas(local_vec, sub, tempSize / ProcNum);
				memcpy(local_vec, sub + tempSize / ProcNum, tempSize / ProcNum * sizeof *sub);
			}
		}		
	}
	
	if (ProcRank)
		MPI_Send(local_vec, tempSize / ProcNum, MPI_INT, root, 0, MPI_COMM_WORLD);
	else {
		end = MPI_Wtime();
		buf = (int*)malloc(sizeof(int) * size);
		int rMove = (tempSize - size);
		memcpy(buf, local_vec + rMove, (tempSize / ProcNum - rMove) * sizeof *local_vec);
		rMove = (tempSize / ProcNum) - rMove;
		for (int i = 1; i < ProcNum; i++) {
			MPI_Recv(sub, tempSize / ProcNum, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
			memcpy(buf + rMove, sub, tempSize / ProcNum * sizeof *sub);
			rMove += tempSize / ProcNum;
		}
		std::cout << "Parallel time : "<< end - start << std::endl;
		start = MPI_Wtime();
		/*
		for (int i = 0; i < size; i++)
			for (int j = 0; j < size - i - 1; j++)
				if (mas[j] > mas[j + 1]) std::swap(mas[j], mas[j + 1]);
		end = MPI_Wtime();
		std::cout << "Normal bubble time : " << end - start << std::endl;
		*/
	}

	MPI_Finalize();
}