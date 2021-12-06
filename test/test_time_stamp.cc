#include <iostream>

#include "time_stamp.h"
#include "utils.h"
// complie: g++ -o test_time_stamp test_time_stamp.cc ../src/time_stamp.cc
// -std=c++14 -I ../include/
void f1() {
  for (int i = 0; i < 1000000; i++)
    std::cout << TimeStamp::Now().TimeToString() << std::endl;
}
// 打印时间一百万次到终端上，耗时30s，主要是打印到终端太耗时了；
int main() {
  utils::TestTime(f1);
  return 0;
}