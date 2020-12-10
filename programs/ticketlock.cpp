
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

volatile int counter = 0;

struct spinlock {
  atomic<unsigned int> current = {0};
  atomic<unsigned int> next_ticket = {0};
  void lock() {
    const unsigned int my_ticket = next_ticket.fetch_add(1);
    while (current.load() != my_ticket)
      ;
  }
  void unlock() { current.store(current.load() + 1); }
} LOCK;

void incr(int amount) {
  for (int i = 0; i < amount; i++) {
    LOCK.lock();
    counter++;
    sleep(1);
    LOCK.unlock();
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2 or argc > 2) {
    cout << "usage: ./ticketlock <num_threads>\n";
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
