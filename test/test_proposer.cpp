/**
 * test_proposer.cpp
 * To test Proposer Class
 * Author: Lijing Wang
 */

#include "proposer.hpp"
namespace mpaxos {
int main(int argc, char** argv) {
  View view;
  PropValue value;
  value.set_data("Hello World!");
  Proposer prop(view, value);  
  return 0;
} 
}

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
