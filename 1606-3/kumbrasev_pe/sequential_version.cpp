#include <vector>
#include <random>
#include <ctime>
#include <cassert>
#include <iostream>
#include <omp.h>
#include "parallel_for.h"


template <typename T>
void insert_sort(std::vector<T> &arr, int i, int step) {
	int j = 0;
	for (int k = step + i; k < arr.size(); k = k + step) {
		j = k;
		while (j > i && arr[j - step] > arr[j]) {
			std::swap(arr[j], arr[j - step]);
			j = j - step;
		}
	}
}

template <typename T>
std::vector<T> shell_sort_openmp(std::vector<T> arr)
{
	assert(arr.size() > 0);
	unsigned h = arr.size() / 2;
	int i;
	int size = arr.size();
	for (; h > 0; h /= 2) {
        #pragma omp parallel for shared(arr, size, h) private (i) default(none)
		for (i = 0; i < h; i++) {
			insert_sort(arr, i, h);
		}
	}
	return arr;
}


template <typename T>
std::vector<T> shell_sort_tbb(std::vector<T> arr)
{
	assert(arr.size() > 0);
	unsigned h = arr.size() / 2;
	int i;
	int size = arr.size();
	for (; h > 0; h /= 2) {
		tbb::parallel_for(tbb::blocked_range<int>(0, h),
		    [&arr, &h](auto&& range) {
			for (auto i = range.begin(); i < range.end(); i++) {
				insert_sort(arr, i, h);
			}
		});
	}
	return arr;
}


template <typename T>
std::vector<T> shell_sort(std::vector<T> arr)
{
	assert(arr.size() > 0);
	unsigned h = arr.size() / 2;
	for (; h > 0; h /= 2)
		for (auto i = 0; i < h; i++) {
			insert_sort(arr, i, h);			
		}
	return arr;
}

int main() {
	constexpr auto test_size = 100000000;
	std::mt19937 gen(time(0));
	std::uniform_int_distribution<> random(0, 100);
	std::vector<int> unsorted_array;


	for (auto i = 0; i < test_size; ++i)
		unsorted_array.emplace_back(random(gen));


	std::cout << std::endl;
	float fTimeStart = omp_get_wtime();
	auto sorted_array = shell_sort(unsorted_array);
	float fTimeStop = omp_get_wtime();

	std::cout << "sequential_version time : " << fTimeStop - fTimeStart << std::endl;

	std::cout << std::endl;
	fTimeStart = omp_get_wtime();
	auto openmp_sorted_array = shell_sort_openmp(unsorted_array);
	fTimeStop = omp_get_wtime();
	std::cout << "openmp_version time : " << fTimeStop - fTimeStart << std::endl;

	std::cout << std::endl;
	fTimeStart = omp_get_wtime();
	auto tbb_sorted_array = shell_sort_tbb(unsorted_array);
	fTimeStop = omp_get_wtime();
	std::cout << "tbb_version time : " << fTimeStop - fTimeStart << std::endl;
	
	/*for (auto&& elem : sorted_array)
		std::cout << elem << " ";
	std::cout << std::endl;
	for (auto&& elem : openmp_sorted_array)
		std::cout << elem << " ";
	std::cout << std::endl;
	for (auto&& elem : tbb_sorted_array)
		std::cout << elem << " ";*/
		
}