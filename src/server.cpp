#include "server.h"
#include <netinet/in.h>
#include <sys/socket.h>

Server::Server() {}

Server::~Server() {}

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
  ret = m_epoller_ptr->AddFd(m_listen_fd, EPOLLIN);
  if (ret == 0) {
    close(m_listen_fd);
    return false;
  }
  fcntl(m_listen_fd, F_SETFL, fcntl(m_listen_fd, F_GETFD, 0) | O_NONBLOCK);
  return true;
}