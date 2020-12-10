/** @file ts_lock.cpp
 *  @brief Implements a simple test-and-set lock
 */
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

int counter = 0;

struct spinlock {
  atomic<bool> l = {false};
  void lock() {
    while (l.exchange(true))
      ;
  }
  void unlock() { l.store(false); }
} LOCK;

void incr(int amount) {
  for (int i = 0; i < amount; i++) {
    LOCK.lock();
    counter++;

    // purposefully hold the lock for longer to generate
    // more contention
    sleep(1);

    LOCK.unlock();
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2 or argc > 2) {
    cout << "usage: ./ts_lock <num_threads>\n";
    exit(0);
  }
  int num_threads = atoi(argv[1]);
  vector<thread> thrList;
  for (int i = 0; i < num_threads; i++) {
    thrList.push_back(thread(incr, 100));
  }
  for (auto &t : thrList) t.join();
  return 0;
}
