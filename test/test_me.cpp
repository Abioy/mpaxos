/**
 * test_proposer.cpp
 * To test Proposer Class
 * Author: Lijing Wang
 */

//#include "internal_types.hpp"
#include "mpaxos.pb.h"
#include <iostream>
namespace mpaxos {
int main(int argc, char** argv) {
  // init Proposer
  PropValue value;
  value.set_data("Hello World!");
  return 0;
} 
}

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
