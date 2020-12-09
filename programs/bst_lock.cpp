#include <iostream>
#include <atomic>
#include <cstdlib>
#include <thread>
#include <vector>
#include <unistd.h>

#define MAX_THREADS (128)

using namespace std;
/*
typedef struct test_and_set_lock {
    atomic<bool> l = {false};
    void lock_lock() {
        while (l.exchange(true));
    }
    void lock_unlock() { l.store(false); }
} lock_t;
*/
/*
typedef struct test_and_test_and_set_lock {
 atomic<bool> l = {false};
  void lock_lock() {
    while (true) {
      if (!l.exchange(true)) return;
      while (l.load())
        ;
    }
  }
  void lock_unlock() { l.store(false); }
} lock_t;
*/
typedef struct ticket_lock {
  atomic<unsigned int> current = {0};
  atomic<unsigned int> next_ticket = {0};
  void lock_lock() {
    const unsigned int my_ticket = next_ticket.fetch_add(1);
    while (current.load() != my_ticket)
      ;
  }
  void lock_unlock() { current.store(current.load() + 1); }
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
  if (root == NULL) return new_node;

  curr->l->lock_lock();
  while (curr != NULL) {
    if (prev != NULL) {
        prev->l->lock_unlock();
    }
    prev = curr;
    if (value < curr->data)
      curr = curr->left;
    else curr = curr->right;
    if (curr != NULL) {

        curr->l->lock_lock();
    }
  }
  if (value < prev->data)
    prev->left = new_node;
  else
    prev->right = new_node;
  prev->l->lock_unlock();
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
//    thrList.push_back(thread(insert, root, 42));
  }
  for (auto &t : thrList) t.join();
}
