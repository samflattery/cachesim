


#include <iostream>
#include <vector>
#include <thread>
#include <stdlib.h>
#include <atomic>
#include <unistd.h>

using namespace std;

int counter = 0;
#define CACHELINE_SIZE (64)
#define MAX_THREADS (128)

typedef struct alignas(CACHELINE_SIZE) padded_atomic_int {
    volatile atomic<unsigned int> data;
} padded_atomic_int_t;

struct spinlock {
    padded_atomic_int_t status[MAX_THREADS];
    volatile atomic<unsigned int> next = {0};
    void init() {
        for (int i = 0; i < MAX_THREADS; i++) status[i].data.store(0);
    }

    int lock() {
        const int my_ind = next.fetch_add(1) % MAX_THREADS;
        while(status[my_ind].data.load() == 1);
        return my_ind;
    }
    void unlock(int my_ind) {
        status[my_ind].data.store(1);
        status[(my_ind + 1) % MAX_THREADS].data.store(0);
    }
} LOCK;

void incr(int amount) {

    for (int i = 0; i < amount; i++) {
        int my_ind = LOCK.lock();
        counter++;
        LOCK.unlock(my_ind);
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2 or argc > 2) {
        cout << "usage: ./spinlock <num_threads>\n";
        exit(0);
    }
    LOCK.init();
    int num_threads = atoi(argv[1]);
    vector<thread> thrList;
    for(int i = 1; i < num_threads; i++) {
        thrList.push_back(thread(incr, 500));
    }
    for (auto &t : thrList) t.join();

    return 0;
}
