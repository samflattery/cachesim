#include <thread>
#include <vector>
#include <stdlib.h>
#include <iostream>

void incr(int *arr, int n, int thread_id) {
  for (int i = thread_id; i < n; i += std::thread::hardware_concurrency()) {
    arr[i]++;
  }
}

int main() {
  int n = 100;
  int *x = (int *) calloc(sizeof(int),  n);
  std::vector<std::thread> threads;

  int num_threads = std::thread::hardware_concurrency() - 1;

  for (int i = 0; i < num_threads; ++i) {
    threads.push_back(std::move(std::thread(incr, x, n, i)));
  }
  incr(x, n, num_threads);

  for (int i = 0; i < num_threads; ++i) {
    threads[i].join();
  }
}
