#pragma once
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool {
public:
  explicit ThreadPool(size_t num);
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;
  ~ThreadPool();
  template <typename F, typename... Args>
  decltype(auto) AddWorkFunc(F &&f, Args &&...args);

private:
  std::queue<std::function<void()>> m_tasksque;
  std::vector<std::thread> m_worker_threads;
  std::mutex m_tasksque_mutex;
  std::condition_variable m_condition;
  bool m_stopflag{false};
};

inline ThreadPool::ThreadPool(size_t num) {
  for (size_t i = 0; i < num; i++) {
    m_worker_threads.emplace_back([this] {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->m_tasksque_mutex);
          this->m_condition.wait(lock, [this] {
            return (this->m_stopflag || !this->m_tasksque.empty());
          });
          if (this->m_stopflag && this->m_tasksque.empty()) {
            return;
          }
          task = std::move(this->m_tasksque.front());
          this->m_tasksque.pop();
        }
        task();
      }
    });
  }
}

inline ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(m_tasksque_mutex);
    m_stopflag = true;
  }
  m_condition.notify_all();
  for (auto &work : m_worker_threads) {
    work.join();
  }
}

template <typename F, typename... Args>
decltype(auto) ThreadPool::AddWorkFunc(F &&f, Args &&...args) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#if _MSVC_LANG <= 201402L
  using return_type = std::result_of_t<F(Args...)>;
#else
  using return_type =
      std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
#endif
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#if __cplusplus <= 201402L
  using return_type = std::result_of_t<F(Args...)>;
#else
  using return_type =
      std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
#endif
#endif
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      [Func = std::forward<F>(f)] { return Func(); });

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(m_tasksque_mutex);
    if (m_stopflag) {
      return std::future<return_type>();
    }
    m_tasksque.emplace([task]() { (*task)(); });
  }
  m_condition.notify_one();
  return res;
}
