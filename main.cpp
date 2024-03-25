#include "src/server.h"

int main() {
  Server s(30010, 8, 3);
  s.Start();
}
