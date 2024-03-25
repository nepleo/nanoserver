#pragma once

#include <atomic>
#include <memory>

#include "epoller.h"
#include "threadpool.h"
#include "timer.h"

class Server {
 public:
  Server(short port, size_t thread_nums, int trig_mode);
  ~Server();
  void Start();
  void Stop();

 private:
  void InitEpoller();
  bool InitSocket();
  void AddClient(int fd, struct sockaddr_in addr);
  void HandleListen();
  // void HandleWrite();
  // void HandelRead();
  void SetNonBlockFd(int fd);

 private:
  std::unique_ptr<Epoller> m_epoller;
  std::unique_ptr<Timer> m_timer;
  std::unique_ptr<ThreadPool> m_threadpool;
  short m_port;
  int m_listen_fd;
  uint32_t m_listen_event;
  uint32_t m_connect_event;
  int timeout_ms{60000};
  int m_trigger_mode;
  std::atomic<bool> m_close{false};
  // 任何连接类型(http, socket)
};