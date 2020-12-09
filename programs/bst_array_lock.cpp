#include <iostream>
#include <atomic>
#include <cstdlib>
#include <thread>
#include <vector>
#include <unistd.h>

#define MAX_THREADS (128)

using namespace std;

typedef struct alignas(64) padded_atomic_int {
  volatile atomic<unsigned int> data;
} padded_atomic_int_t;

typedef struct array_lock {
  padded_atomic_int_t status[MAX_THREADS];
  volatile atomic<unsigned int> next = {0};
  void init() {
    status[0].data.store(0);
    for (int i = 1; i < MAX_THREADS; i++) status[i].data.store(1);
  }

  int lock_lock() {
    const int my_ind = next.fetch_add(1) % MAX_THREADS;
    while (status[my_ind].data.load() == 1)
      ;
    return my_ind;
  }
  void lock_unlock(int my_ind) {
    status[my_ind].data.store(1);
    status[(my_ind + 1) % MAX_THREADS].data.store(0);
  }
} lock_t;


typedef struct node {
  int data;
  struct node *left = NULL;
  struct node *right = NULL;
  lock_t *l;
} node_t;


node_t* insert(node_t *root, int value) {
  node_t *curr = root, *prev = NULL;
  node_t *new_node = (node_t*)malloc(sizeof(node_t));
  new_node->data = value;
  new_node->l = (lock_t*)malloc(sizeof(lock_t));
  new_node->l->init();
  if (root == NULL) return new_node;

  int curr_lock_ind, prev_lock_ind;

  curr_lock_ind = curr->l->lock_lock();
  while (curr != NULL) {
    if (prev != NULL) {
        prev->l->lock_unlock(prev_lock_ind);
    }
    prev = curr;
    prev_lock_ind = curr_lock_ind;
    if (value < curr->data)
      curr = curr->left;
    else curr = curr->right;
    if (curr != NULL) {
        curr_lock_ind = curr->l->lock_lock();
    }
  }
  if (value < prev->data)
    prev->left = new_node;
  else
    prev->right = new_node;
  prev->l->lock_unlock(prev_lock_ind);
  return root;
}

void print2DUtil(node_t *root, int space)
{
    // Base case
    if (root == NULL)
        return;
    // Increase distance between levels
    space += 10;
    // Process right child first
    print2DUtil(root->right, space);
    // Print current node after space
    // count
    cout<<endl;
    for (int i = 10; i < space; i++)
        cout<<" ";
    cout<<root->data<<"\n";
    // Process left child
    print2DUtil(root->left, space);
}

void insert_many(node_t* root, int count) {
  for (int i = 0; i < count; i++) {
    int r = rand();
    insert(root, r);
  }

}

int main() {
  srand(42);
  node_t* root = insert(NULL, rand());

  int num_threads = 8;
  vector<thread> thrList;
  for (int i = 1; i < num_threads; i++) {
    thrList.push_back(thread(insert_many, root, 1000));
  }
  for (auto &t : thrList) t.join();
}
