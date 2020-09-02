#include <iostream>


/**
 * https://www.zhihu.com/question/24671324 
 * 为什么下列函数可以编译通过，并指定编译器能链接到正确的类型？
 * 从答案内，我们可以看到，问题的关键在于理解编译器如何生成全局唯一的函数签名
 * 
 * 
 * 
 */


// 函数名比较怪，在符号表内更容易识别
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
  std::cout << f88293(7) << std::endl;
}
