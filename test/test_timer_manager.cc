#include <unistd.h>

#include <iostream>

#include "timer_manager.h"
#include "utils.h"
using namespace std;
TimerManager timer_manager;
void TimerCallback(int n) { cout << n << " time on!" << endl; }
// 测试计时一次
void Test1() {
  for (int i = 0; i < 8; i++) {
    std::function<void()> f = std::bind(TimerCallback, i);
    cout << "Timer地址：" << timer_manager.AddTimer(0, 5000, f) << endl;
    cout << i << "号定时器已经添加" << endl;
    usleep(1000000);
  }
  timer_manager.CheckAndHandle();
}
// 测试计时多次
void Test2() {
  for (int i = 0; i < 2; i++) {
    std::function<void()> f = std::bind(TimerCallback, i);
    cout << "Timer地址：" << timer_manager.AddTimer(2, 5000, f)
         << endl;  // repeated_times为2,则该定时器总共计时3次
    cout << i << "号定时器已经添加" << endl;
    usleep(1000000);
  }
  while (1) timer_manager.CheckAndHandle();
}

int main() {
  cout << "------------测试计时一次----------------" << endl;
  utils::TestTime(Test1);
  cout << "------------测试计时多次----------------" << endl;
  utils::TestTime(Test2);
  return 0;
}