/**
 * 模版偏特化的一些小 demo.
 */

#include <iostream>
#include <memory>

template <typename T, typename U>
struct X {
  void id() { std::cout << 0 << std::endl; }
};

template <typename T>
struct X<T, T> {
  void id() { std::cout << 1 << std::endl; }
};

template <typename T>
struct X<T*, T> {
  void id() { std::cout << 2 << std::endl; }
};

template <typename T>
struct X<T, T*> {
  void id() { std::cout << 3 << std::endl; }
};

template <typename U>
struct X<U, int> {
  void id() { std::cout << 4 << std::endl; }
};

template <typename U>
struct X<U*, int> {
  void id() { std::cout << 5 << std::endl; }
};

template <typename U, typename T>
struct X<U*, T*> {
  void id() { std::cout << 6 << std::endl; }
};

template <typename U, typename T>
struct X<U, T*> {
  void id() { std::cout << 7 << std::endl; }
};

template <typename T>
struct X<std::unique_ptr<T>, std::shared_ptr<T>> {
  void id() { std::cout << 8 << std::endl; }
};



// 以下特化，分别对应哪个偏特化的实例？
// 此时偏特化中的T或U分别是什么类型？

X<float*, int> v0;
X<double*, int> v1;
X<double, double> v2;
X<float*, double*> v3;
// 匹配冲突：1 vs 6
// X<float*, float*> v4;
X<double, float*> v5;
X<int, double*> v6;
// 匹配冲突：5 vs 2
// X<int*, int> v7;
X<double*, double> v8;

int main(int argc, char* argv[]) {
  v0.id();
  v1.id();
  v2.id();
  v3.id();
  // v4.id();
  v5.id();
  v6.id();
  // v7.id();
  v8.id();
}