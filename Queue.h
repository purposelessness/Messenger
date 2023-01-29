#ifndef MESSENGER__QUEUE_H_
#define MESSENGER__QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class Queue {
  struct Node {
    T value;
    std::unique_ptr<Node> next;
  };

 public:
  Queue() : _head(std::make_unique<Node>()), _tail(_head.get()) {}

  void push(T value);
  T waitAndPop();
  [[nodiscard]] bool empty() const;

 private:
  Node* getTail();

  std::unique_ptr<Node> _head;
  Node* _tail;
  mutable std::mutex _hm;
  mutable std::mutex _tm;
  std::condition_variable _cv;
};

template <typename T>
void Queue<T>::push(T value) {
  std::unique_ptr<Node> node{};
  {
    std::scoped_lock lock(_tm);
    _tail->value = std::move(value);
    _tail->next = std::move(node);
    _tail = _tail->next.get();
  }
  _cv.notify_one();
}

template <typename T>
T Queue<T>::waitAndPop() {
  std::unique_lock lock(_hm);
  _cv.wait(lock, [this]() { return _head.get() != getTail(); });
  auto val = std::move(_head->value);
  _head = std::move(_head->next);
  return val;
}

template <typename T>
bool Queue<T>::empty() const {
  std::scoped_lock lock(_hm, _tm);
  return _head.get() == _tail;
}

template <typename T>
Queue<T>::Node* Queue<T>::getTail() {
  std::scoped_lock lock(_tm);
  return _tail;
}

#endif  // MESSENGER__QUEUE_H_