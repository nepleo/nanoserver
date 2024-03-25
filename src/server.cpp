#include "server.h"
#include <netinet/in.h>
#include <sys/socket.h>

Server::Server(short port, size_t thread_nums, int trig_mode)
    : m_port(port), m_threadpool(std::make_unique<ThreadPool>(8)), m_trigger_mode(trig_mode) {
  m_timer = std::make_unique<Timer>();
  m_epoller = std::make_unique<Epoller>();
  InitEpoller();
  m_close = !InitSocket();
}

Server::~Server() { close(m_listen_fd); }

void Server::Stop() { m_close = true; }

bool Server::InitSocket() {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(m_port);
  struct linger opt_linger = {0};
  opt_linger.l_onoff = 1;
  opt_linger.l_linger = 1;
  m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_listen_fd < 0) {
    return false;
  }
  int ret = setsockopt(m_listen_fd, SOL_SOCKET, SO_LINGER, &opt_linger, sizeof(opt_linger));
  if (ret < 0) {
    close(m_listen_fd);
    return false;
  }
  int optval = 1;
  ret = setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
  if (ret == -1) {
    close(m_listen_fd);
    return false;
  }
  ret = bind(m_listen_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    close(m_listen_fd);
    return false;
  }
  ret = listen(m_listen_fd, 5);
  if (ret < 0) {
    close(m_listen_fd);
    return false;
  }
  ret = m_epoller->AddFd(m_listen_fd, m_listen_event | EPOLLIN);
  if (ret == 0) {
    close(m_listen_fd);
    return false;
  }
  SetNonBlockFd(m_listen_fd);
  return true;
}

void Server::InitEpoller() {
  m_listen_event = EPOLLRDHUP;
  m_connect_event = EPOLLONESHOT | EPOLLRDHUP;
  if (m_trigger_mode == 0) {
  } else if (m_trigger_mode == 1) {
    m_connect_event |= EPOLLET;
  } else if (m_trigger_mode == 2) {
    m_listen_event = EPOLLET;
  } else if (m_trigger_mode == 3) {
    m_listen_event |= EPOLLET;
    m_connect_event |= EPOLLET;
  } else {
    m_listen_event |= EPOLLET;
    m_connect_event |= EPOLLET;
  }
}

void Server::Start() {
  int time_ms = -1;
  while (!m_close) {
    if (timeout_ms > 0) {
      time_ms = m_timer->GetNextTick();
    }
    int event_cnt = m_epoller->Wait(time_ms);
    for (int i = 0; i < event_cnt; i++) {
      int fd = m_epoller->GetEventFd(i);
      uint32_t events_type = m_epoller->GetEvents(i);
      if (fd == m_listen_fd) {
        // 处理监听
      } else if (events_type & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        // 关闭连接
      } else if (events_type & EPOLLIN) {
        // 读取数据
      } else if (events_type & EPOLLOUT) {
        // 处理写数据
      } else {
      }
    }
  }
}

void Server::SetNonBlockFd(int fd) { fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK); }

void Server::HandleListen() {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  do {
    int fd = accept(m_listen_fd, (struct sockaddr *)&addr, &len);
    if (fd <= 0) {
      return;
    }
    AddClient(fd, addr);
  } while (m_listen_event & EPOLLET);
}

void Server::AddClient(int fd, sockaddr_in addr) {
  if (timeout_ms > 0) {
    // 加入超时定时器
  }
  m_epoller->AddFd(fd, EPOLLIN | m_connect_event);
  SetNonBlockFd(fd);
}