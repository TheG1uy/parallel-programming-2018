#include <iostream>
#include <string>
#include "mpi.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include <vector>
#define root 0


struct Region {
	int number;
	const uchar color;
	Region(int _num, uchar _col) : number(_num), color(_col) {}
};
std::vector<Region> countRegions(int *src, int rows, int cols) {
	std::vector<std::vector<Region>> buf;
	int regionCounter = 0;
	buf.resize(rows);

	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			buf[i].push_back(Region(-1, (src[i * cols + j]) + 10 > 255 ? 255 : 0));
	buf[0][0].number = 1;
	regionCounter++;
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j) {
			if (buf[i][j].number == -1) {
				int k = j;
				while (k < cols && buf[i][k].number == -1) 	k++;

				if (k < cols && buf[i][k].color == buf[i][j].color)
					buf[i][j].number = buf[i][k].number;
				else buf[i][j].number = ++regionCounter;
			}
			uchar col = buf[i][j].color;
			int num = buf[i][j].number;
			if (i && buf[i - 1][j].color == col) buf[i - 1][j].number = num;
			if (i < rows - 1 && buf[i + 1][j].color == col) buf[i + 1][j].number = num;
			if (j && buf[i][j - 1].color == col) buf[i][j - 1].number = num;
			if (j < cols - 1 && buf[i][j + 1].color == col) buf[i][j + 1].number = num;
		}
	std::vector<Region> buf_r;
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			buf_r.push_back(buf[i][j]);

	return buf_r;
}
int main(int argv, char *argc[]) {
	int* buf = nullptr;
	int* sub = nullptr;
	int src_size = 0;
	int* local_vec = nullptr;
	int ProcNum;
	int ProcRank = 0;
	int tempSize;
	MPI_Status status;
	double start, end;
	int* displs = nullptr;
	int* scounts = nullptr;
	cv::Mat img = cv::imread("test4.png");
	int width = img.cols, height = img.rows;
	int* src = (int*)malloc(img.rows * img.cols * sizeof(int));
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j) {
			src[src_size++] = (int)img.at<uchar>(i, j);
		}



	MPI_Init(&argv, &argc);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	if (ProcNum > 1) {
		if (!ProcRank) {
			(height % ProcNum) ? tempSize = (height + ProcNum - 1) / ProcNum * ProcNum : tempSize = height;
			buf = (int*)malloc(tempSize * width * sizeof(int));
			memcpy(buf, src, height * width * sizeof *src);
			for (int i = height * width; i < tempSize * width; ++i)
				buf[i] = 0;
			start = MPI_Wtime();
			displs = (int*)malloc(sizeof(int) * ProcNum);
			scounts = (int*)malloc(sizeof(int) * ProcNum);
			for (int i = 1; i < ProcNum - 1; ++i) {
				displs[i] = (i * tempSize / ProcNum - 1) * width;
				scounts[i] = (tempSize / ProcNum + 2) * width;
			}
			displs[0] = 0;
			displs[ProcNum - 1] = ((ProcNum - 1) * tempSize / ProcNum - 1) * width;
			scounts[0] = (tempSize / ProcNum + 1) * width;
			scounts[ProcNum - 1] = (tempSize / ProcNum + 1) * width;

		}
		MPI_Bcast(&tempSize, 1, MPI_INT, root, MPI_COMM_WORLD);
		int send_count = 0;
		if (ProcRank && ProcRank < ProcNum - 1) {
			send_count = (tempSize / ProcNum + 2) * width;
			local_vec = (int*)malloc(sizeof(int) * send_count);
		}
		else {
			send_count = (tempSize / ProcNum + 1) * width;
			local_vec = (int*)malloc(sizeof(int) * send_count);
		}
		MPI_Scatterv(buf, scounts, displs, MPI_INT, local_vec, send_count, MPI_INT, root, MPI_COMM_WORLD);
		std::vector<Region> regions = countRegions(local_vec, tempSize / ProcNum + 2, width);
		free(local_vec);
		local_vec = (int*)malloc(sizeof(int) * (tempSize / ProcNum) * width);

		if (ProcRank) {
			for (int i = 0; i < tempSize / ProcNum * width; ++i)
				local_vec[i] = regions[i + width].number;
		}
		else {
			for (int i = 0; i < tempSize / ProcNum * width; ++i)
				local_vec[i] = regions[i].number;
			free(buf);
			buf = (int*)malloc(sizeof(int) * tempSize * width);
		}

		MPI_Gather(local_vec, tempSize / ProcNum * width, MPI_INT, buf, tempSize / ProcNum * width, MPI_INT, root, MPI_COMM_WORLD);
		if (!ProcRank) {
			/*int k = 0;
			for (int i = 0; i < height; ++i, std::cout << std::endl)
				for (int j = 0; j < width; ++j) {
					std::cout << buf[k++] << " ";
				}*/
			end = MPI_Wtime();
			std::cout << end - start << std::endl;
		}
		
					
	}
	else {
		start = MPI_Wtime();
		std::vector<Region> regions = countRegions(src, height, width);
		/*int k = 0;
		for (int i = 0; i < height; ++i, std::cout << std::endl)
			for (int j = 0; j < width; ++j) {
				std::cout << regions[k++].number << " ";
			}*/
		end = MPI_Wtime();
		std::cout << std::endl << end - start << std::endl;
	}
	           
	MPI_Finalize();


}