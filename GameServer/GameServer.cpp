#include "pch.h"
#include <thread>


void HelloThread() { std::cout << "Hello, threaaaaeead!\n"; }

int main() {
  std::thread t(HelloThread);
  std::cout << "Hello World!\n";
  t.join();
}