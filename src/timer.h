#pragma once

#include <chrono>
#include <functional>
#include <queue>

using TimeoutCallBack = std::function<void()>;

struct TimerNode {
  unsigned int id;
  std::chrono::high_resolution_clock::time_point expires;
  TimeoutCallBack cb;
  bool operator<(const TimerNode &t) { return t.expires < t.expires; }
};

// 最小堆定时器
class Timer {
 public:
  Timer();
  ~Timer();
  void Add(unsigned int id, int timeout, const TimeoutCallBack &cb);
  void Clear();
  void Tick();
  void Pop();
  int GetNextTick();

 private:
  void Adjust(size_t index);
  void SiftUp(size_t index);
  void SiftDown(size_t index);
  void SwapNode(size_t i, size_t j);
  void Del(size_t i);

 private:
  std::vector<TimerNode> m_heap;
  std::unordered_map<unsigned int, size_t> m_ref;
};
