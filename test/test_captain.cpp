/**
 * test_captain.cpp
 * To test Proposer Acceptor Captain Commo(No Message exchange) 
 * Author: Lijing Wang
 */

#include "captain.hpp"
#include "commo.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <chrono>

namespace mpaxos {

void foo() {
  std::cout << "FOO" << std::endl;
}
void bar(int x) {
  std::cout << "BAR " << x << std::endl;
}
// client to commit value
void client_commit(Captain * captain) {
  std::string line;
  node_id_t node_id = captain->get_node_id();
  std::string filename = "values/client_" +  std::to_string(node_id);
  std::cout << "FileName " << filename << std::endl;
  std::ifstream value_file (filename);
  if (value_file.is_open())
  {
    while (getline(value_file, line)) {
      std::cout << "before commit Node " << node_id << " Value " << line << std::endl;
      captain->commit_value(line); 
      std::cout << "commit this value succeed " << line << std::endl;
    }
//    if (line == "") 
//      value_file << "Hello World From Node" + std::to_string(node_id); 
    value_file.close();
    std::cout << "Close File client_" << node_id << std::endl;
  }
  else 
    std::cout << "Unable to open file" << std::endl; 
}

int main(int argc, char** argv) {
  std::cout << "START " << std::endl;

  int num = 5;
  if (argc > 1)
    num = atoi(argv[1]);
  std::set<node_id_t> nodes;
  // init all nodes set
  for (int i = 0; i < num; i++) 
    nodes.insert(i);

  std::vector<Captain *> captains;
  std::vector<View *> views;
  // init all view & captain
  for (int i = 0; i < num; i++) {
    views.push_back(new View(i, nodes));
    captains.push_back(new Captain(*views[i]));
  }
  
  Commo commo(captains);
  std::vector<std::thread *> clients; 
  // set commo for every captain & init a new client thread
  for (int i = 0; i < num; i++) {
    captains[i]->set_commo(&commo);
//    client_commit(captains[i]);
//    clients.push_back(new std::thread(client_commit, captains[i]));
  }

  for (int i = 0; i < num; i++)
    client_commit(captains[i]);
//  std::this_thread::sleep_for(std::chrono::seconds(100));

//  for (int i = 0; i < num; i++)
//    clients[i]->join();

  std::cout << "END " << std::endl;
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
