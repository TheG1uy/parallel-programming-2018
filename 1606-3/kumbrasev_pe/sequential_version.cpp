#include <vector>
#include <random>
#include <ctime>
#include <cassert>
#include <iostream>

template <typename T>
std::vector<T> shell_sort(std::vector<T> arr)
{
	assert(arr.size() > 0);
	auto end = arr.size() - 1;

	unsigned h = end / 2;
	for (h = 1; h <= end / 9; h = 3 * h + 1)
		;
	std::cout << h << std::endl;
	for (; h > 0; h /= 3)
		for (auto i = h; i <= end; i++)
		{
			unsigned j = i;
			T v = arr[i];
			while (j >= h && v < arr[j - h])
			{
				arr[j] = arr[j - h];
				j -= h;
			}
			arr[j] = v;
		}
	return arr;
}

int main() {
	constexpr auto test_size = 10;
	std::mt19937 gen(time(0));
	std::uniform_int_distribution<> random(-100, 100);
	std::vector<int> unsorted_array;

	for (auto i = 0; i < test_size; ++i)
		unsorted_array.emplace_back(random(gen));

	for (auto&& elem : unsorted_array)
		std::cout << elem << " ";

	std::cout << std::endl;

	auto sorted_array = shell_sort(unsorted_array);

	for (auto&& elem : sorted_array)
		std::cout << elem << " ";
}