
#include <iostream>
#include <vector>
#include <thread>
#include <stdlib.h>
#include <atomic>
#include <unistd.h>

using namespace std;

int counter = 0;

struct spinlock {
    atomic<bool> l = {false};
    void lock() {
        while(true) {
            if (!l.exchange(true)) return;
            while(l.load());
        }
    }
    void unlock() { l.store(false); }
} LOCK;

void incr(int amount) {

    for (int i = 0; i < amount; i++) {
        LOCK.lock();
        counter++;
        sleep(1); // purposefully hold the lock for longer
        LOCK.unlock();
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2 or argc > 2) {
        cout << "usage: ./spinlock <num_threads>\n";
        exit(0);
    }
    int num_threads = atoi(argv[1]);
    vector<thread> thrList;
    for(int i = 1; i < num_threads; i++) {
        thrList.push_back(thread(incr, 10));
    }
    for (auto &t : thrList) t.join();
    return 0;
}
