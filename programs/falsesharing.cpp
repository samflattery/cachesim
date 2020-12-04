#include <stdlib.h>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

#define ARRLEN 100000

typedef struct padded_int {
  int data;
} padded_int_t;

static padded_int_t arr[ARRLEN];  // use static to 0-init

void add(int rem, int mod) {
  for (int i = 0; i < ARRLEN; i++) {
    if (i % mod == rem) arr[i].data++;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2 or argc > 2) {
    cout << "usage: ./falsesharing <num_threads>\n";
    exit(0);
  }
  const int num_threads = atoi(argv[1]);
  vector<thread> thrList;
  for (int i = 1; i < num_threads; i++) {
    thrList.push_back(thread(add, i - 1, num_threads));
  }
  for (auto &t : thrList) t.join();
  return 0;
}
