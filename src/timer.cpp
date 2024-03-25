#include "timer.h"

void Timer::Clear() {
  m_heap.clear();
  m_ref.clear();
}

Timer::Timer() { m_heap.reserve(100); }

Timer::~Timer() { Clear(); }

void Timer::Add(unsigned int id, int timeout, const TimeoutCallBack &cb) {
  size_t i;
  if (m_ref.count(id) == 0) {
    i = m_heap.size();
    m_ref[id] = i;
    m_heap.push_back({id, std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout), cb});
    SiftUp(i);
  } else {
    i = m_ref[id];
    m_heap[i].expires = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout);
    m_heap[i].cb = cb;
    Adjust(i);
  }
}

void Timer::SiftUp(size_t index) {
  while (index > 0) {
    size_t parent = (index - 1) / 2;
    if (m_heap[index] < m_heap[parent]) {
      SwapNode(index, parent);
      index = parent;
    } else {
      break;
    }
  }
}

void Timer::SiftDown(size_t index) {
  size_t size = m_heap.size();
  while (index < size) {
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    size_t smallest = index;
    if (left < size && m_heap[left] < m_heap[right]) {
      smallest = left;
    }
    if (right < size && m_heap[right] < m_heap[smallest]) {
      smallest = right;
    }
    if (smallest != index) {
      SwapNode(index, smallest);
      index = smallest;
    } else {
      break;
    }
  }
}

void Timer::Adjust(size_t index) {
  if (index > 0 && m_heap[index] < m_heap[(index - 1) / 2]) {
    SiftUp(index);
  } else {
    SiftDown(index);
  }
}

void Timer::SwapNode(size_t i, size_t j) {
  std::swap(m_heap[i], m_heap[j]);
  m_ref[m_heap[i].id] = i;  // 交换后记住索引
  m_ref[m_heap[j].id] = j;
}

void Timer::Del(size_t i) {
  size_t index = i;
  size_t n = m_heap.size() - 1;
  if (index < n) {
    SwapNode(index, n);
    SiftDown(index);
  }
  m_ref.erase(m_heap.back().id);
  m_heap.pop_back();
}

void Timer::Pop() { Del(0); }

void Timer::Tick() {
  if (m_heap.empty()) {
    return;
  }
  while (!m_heap.empty()) {
    TimerNode node = m_heap.front();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(node.expires - std::chrono::high_resolution_clock::now())
            .count() > 0) {
      break;
    }
    node.cb();
    Pop();
  }
}

int Timer::GetNextTick() {
  Tick();
  size_t res = -1;
  if (!m_heap.empty()) {
    res = std::chrono::duration_cast<std::chrono::milliseconds>(m_heap.front().expires -
                                                                std::chrono::high_resolution_clock::now())
              .count();
    if (res < 0) {
      res = 0;
    }
  }
  return res;
}