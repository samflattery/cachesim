#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

typedef int datatype;

vector<datatype> v;
int n, num_threads;

void gen_array() {
  srand(time(NULL));
  for (int i = 0; i < n; i++) {
    v[i] = rand();
  }
}

void sort_part(int start_index) {
  sort(v.begin() + start_index, v.begin() + start_index + n / num_threads);
}

void merge() {
  vector<datatype> v_res;
  v_res.resize(n);
  vector<int> indicies(num_threads, 0);

  for (int v_res_index = 0; v_res_index < n; v_res_index++) {
    datatype min_value = INT_MAX;
    int min_ind = -1;
    for (int i = 0; i < num_threads; i++) {
      int ind = indicies[i];
      if (ind >= n / num_threads) continue;
      if (v[n / num_threads * i + ind] < min_value) {
        min_value = v[n / num_threads * i + ind];
        min_ind = i;
      }
    }
    v_res[v_res_index] = min_value;
    indicies[min_ind]++;
  }
  v = vector<datatype>(v_res);
}

int main(int argc, char *argv[]) {
  if (argc < 3 or argc > 3) {
    cout << "usage: ./sort <num_threads> <num_elements>\n";
    exit(0);
  }
  n = atoi(argv[2]);
  num_threads = atoi(argv[1]);
  if (n % num_threads != 0) {
    cout << "num_threads must be multiple of num_elements\n";
    exit(0);
  }

  v.resize(n);

  gen_array();

  vector<thread> thrList;
  for (int i = 1; i < num_threads; i++) {
    thrList.push_back(thread(sort_part, n / num_threads * i));
  }
  sort_part(0);
  for (auto &t : thrList) {
    t.join();
  }

  // don't actually merge because then t0 has too many ops compared to other
  //    merge();

  return 0;
}
