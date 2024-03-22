#include "epoller.h"

Epoller::Epoller(int max_event) : m_events(max_event) {
  m_epollfd = epoll_create(1024);
  assert(m_epollfd >= 0);
}

Epoller::~Epoller() { close(m_epollfd); }

bool Epoller::AddFd(int fd, uint32_t events_type) {
  if (fd < 0) {
    return false;
  }
  epoll_event ev{0};
  ev.data.fd = fd;
  ev.events = events_type;
  return epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::DelFd(int fd) {
  if (fd < 0) {
    return false;
  }
  epoll_event ev{0};
  return epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &ev) == 0;
}

bool Epoller::ModFd(int fd, uint32_t events_type) {
  if (fd < 0) {
    return false;
  }
  epoll_event ev{0};
  ev.data.fd = fd;
  ev.events = events_type;
  return epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &ev) == 0;
}

int Epoller::Wait(int timeout_ms) {
  int events_count = epoll_wait(m_epollfd, m_events.data(), (int)m_events.size(), timeout_ms);
  if (events_count == -1 && errno == ENOMEM) {
    m_events.resize(m_events.size() * 2);
    events_count = epoll_wait(m_epollfd, m_events.data(), (int)m_events.size(), timeout_ms);
  }
  return events_count;
}

int Epoller::GetEventFd(size_t i) const {
  assert(i < m_events.size() && i >= 0);
  return m_events[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const { return m_events[i].events; }
