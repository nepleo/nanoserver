#pragma once

#include <memory>

#include "epoller.h"

class Server {
 public:
  Server();
  ~Server();
  void Start();

 private:
  void InitServer();
  void InitEpoller();
  bool InitSocket();

 private:
  std::unique_ptr<Epoller> m_epoller_ptr;
  short m_port{30010};
  int m_listen_fd;
};