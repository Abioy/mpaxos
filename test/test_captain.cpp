/**
 * test_captain.cpp
 * To test Proposer Acceptor Captain Commo(No Message exchange) 
 * Author: Lijing Wang
 */

//#include "captain.hpp"
//#include <iostream>
//#include <thread>
//#include <cstdlib>
//namespace mpaxos {
//void foo() {
//  std::cout << "FOO" << std::endl;
//}
//void bar(int x) {
//  std::cout << "BAR " << x << std::endl;
//}
//int main(int argc, char** argv) {
//  std::cout << "START " << std::endl;
//  std::thread first(foo);
////  std::thread second(bar, 0);
////  first.join();
////  second.join();
//  std::cout << "END " << std::endl;
//  return EXIT_SUCCESS;
//}
//} // namespace mpaxos
//
//int main(int argc, char** argv) {
//  return mpaxos::main(argc, argv);
//}
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
using namespace std;
void task1(std::string msg){
  cout << "task1 says: " << msg << endl;
}
int main() { 
  std::thread t1(task1, "hello"); 
  t1.join();
  return 0;
}
