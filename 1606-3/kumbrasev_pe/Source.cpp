#include "mpi.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <ctime>
#include <cmath>



int main(int argv, char *argc[]) {
	srand(time(0));
	int diff1 = 0, diff2 = 0, diffsum = 0;
	int localSize = 0;
	int newSize;
	int ProcNum;
	int rank;
	double start, end;
	char *str;
	MPI_Status status;
	std::string st1;
	std::string st2;


	for (int k = 0; k < 1000; k++) {
		st1 += 'a' + rand() % 26;
		st2 += 'a' + rand() % 26;
	}
	diff1 = abs((int)st1.size() - (int)st2.size());
	st1.size() > st2.size() ? st1.resize(st2.size()) : st2.resize(st1.size());
	int n = st1.size();
	str = new char();

	MPI_Init(&argv, &argc);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0) {
		start = MPI_Wtime();
		localSize = (n + ProcNum - 1) / ProcNum;
		newSize = (localSize * ProcNum);
		st1 += std::string((newSize - n), '0');
		st2 += std::string((newSize - n), '0');
		std::string buf = "";
		str = new char[2 * newSize + 1];
		for (int i = 0; i < newSize; i += localSize) 
			buf += st1.substr(i, localSize) + st2.substr(i, localSize);
		
		
		strcpy_s(str, newSize * 2 + 1, buf.c_str());
	}


	MPI_Bcast(&localSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
	char *local_str = (char *)malloc(localSize * 2 * sizeof(char));
	MPI_Scatter(str, localSize * 2, MPI_CHAR, local_str, localSize * 2, MPI_CHAR, 0, MPI_COMM_WORLD);
	local_str[localSize * 2] = '\0';
	for (int i = 0; i < localSize; i++)
		diff2 += str[i] != str[i + localSize] ? 1 : 0;
	MPI_Reduce(&diff2, &diffsum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		end = MPI_Wtime();
		printf("Work time  :  %f\n", end - start);
		printf("Difference  :  %d", diffsum + diff1);
	}
	
	MPI_Finalize();
	
}