#include <iostream>

/**
 * https://www.zhihu.com/question/24671324
 * 为什么下列函数可以编译通过，并指定编译器能链接到正确的类型？
 * 
 * 1. T* 会被编译器认为是特化类型而优先匹配.
 * 2. 特化类型如果能匹配, 则不会匹配通用的模版类型.
 */

template <typename T>
int f6677(T) {
  return 1;
}

template <typename T>
int f6677(T*) {
  return 2;
}

template <typename T>
std::string f88293(T) {
  return "Template";
}

std::string f88293(int&) { return "Nontemplate"; }

int main() {
  std::cout << f6677(0) << std::endl;
  std::cout << f6677((int*)0) << std::endl;
  int x = 7;
  std::cout << f88293(x) << std::endl;
}
