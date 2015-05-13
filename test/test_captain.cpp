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
//  std::cout << "FileName " << filename << std::endl;
  std::ifstream value_file (filename);
  if (value_file.is_open())
  {
    while (getline(value_file, line)) {
//      std::cout << "** client_commit: before commit Node " << node_id << " Value " << line << std::endl;
      LOG_INFO("** Before call <captain->client_commit>  --NodeID %u (value):%s", node_id, line.c_str());
      captain->commit_value(line); 
      LOG_INFO("** After call <captain->client_commit>  --NodeID %u (value):%s", node_id, line.c_str());
    }
//    if (line == "") 
//      value_file << "Hello World From Node" + std::to_string(node_id); 
    value_file.close();
    LOG_INFO("** Close File  --NodeID %u", node_id);
  }
  else 
    std::cout << "** Unable to open file" << std::endl; 
}

int main(int argc, char** argv) {
  LOG_INFO("** START **");

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

  for (int i = num -1 ; i >=0; i--) {
    LOG_INFO("***********************************************************************");
    LOG_INFO("** (Client):%d Commit Start", i);
    client_commit(captains[i]);
    LOG_INFO("** (Client):%d Commit END", i);
    LOG_INFO("***********************************************************************");
  }
//  std::this_thread::sleep_for(std::chrono::seconds(100));

//  for (int i = 0; i < num; i++)
//    clients[i]->join();

  LOG_INFO("** END **");
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
