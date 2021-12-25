/*
 * @Author: lei
 * @Description: 从配置文件拉取配置
 * @FilePath: /MyMuduo/include/config.h
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>

class Config {
 public:
  bool PullConfig(const std::string& filename) {
    std::ifstream s(filename, s.binary | s.out);
    if (!s.is_open()) {
      std::cout << "failed to open " << filename << std::endl;
      return false;
    }
    while (getline(s, buf)) Parse_line();
    return true;
  }
  void OutputConfig() {
    for (auto& v : mp) {
      std::cout << "key=" << v.first << ",value=" << v.second << std::endl;
    }
  }

  std::unordered_map<std::string, std::string> mp;

 private:
  std::string buf;

  void Parse_line();
};
void Config::Parse_line() {
  std::smatch matches;
  std::regex regex1(
      "^([^ =]*)[^\\d]*(\\d+.*)");  // \d是特殊字符，所以这里要转义
  if (std::regex_search(buf, matches, regex1)) {
    mp[matches.str(1)] = matches.str(2);
  }
}

#endif  // CONFIG_H