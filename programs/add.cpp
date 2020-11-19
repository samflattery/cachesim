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
  int thread_id = 0;
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
    /* std::cout << thread_id << std::endl; */
    threads.push_back(std::move(std::thread(incr, x, n, thread_id)));
    thread_id++;
  }
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
    threads[i].join();
  }
}
