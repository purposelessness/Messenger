#include <iostream>

#include "Queue.h"

int main() {
  Queue<int> queue;
  queue.push(10);
  std::cout << queue.waitAndPop();
  return 0;
}
