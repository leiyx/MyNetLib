#include <iostream>

#include "event_loop.h"
#include "utils.h"
using namespace std;
void TimerCallback() { std::cout << "time on!" << std::endl; }
void Test() {
  EventLoop loop;
  loop.RunAfter(2, 3000, TimerCallback);
  loop.Loop();
}
int main() {
  Test();
  return 0;
}