#pragma once

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

class Epoller {
public:
  explicit Epoller(int max_event = 1024);
  ~Epoller();
  bool AddFd(int fd, uint32_t events_type);
  bool ModFd(int fd, uint32_t events_type);
  bool DelFd(int fd);
  int Wait(int timeout_ms = -1);
  int GetEventFd(size_t i) const;
  uint32_t GetEvents(size_t i) const;

private:
  int m_epollfd;
  std::vector<struct epoll_event> m_events;
};
