#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <thread>
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------
// PROGRAMACION GENERICA Y PROGRAMACION PARALELA
// ---------------------------------------------------------

template<typename T, typename U, typename Func>
void parallel_map(const std::vector<T>& input, std::vector<U>& output, Func f, int num_threads = std::thread::hardware_concurrency()) {
    if (num_threads <= 0) num_threads = 4;
    int n = input.size();
    output.resize(n);
    if (n == 0) return;
    int chunk_size = std::ceil((double)n / num_threads);
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = std::min(start + chunk_size, n);
        if (start < n) {
            threads.emplace_back([start, end, &input, &output, f]() {
                for (int j = start; j < end; ++j) {
                    output[j] = f(input[j], j);
                }
            });
        }
    }
    for (auto& t : threads) {
        t.join();
    }
}

#endif // UTILS_H
