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

void do_sth(slot_id_t slot_id, std::string& data) {
  LOG_INFO("HAHA slot_id:%llu value:%s", slot_id, data.c_str());
}

// client to commit value
void client_commit_file(Captain * captain) {
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
      LOG_INFO("** After  call <captain->client_commit>  --NodeID %u (value):%s", node_id, line.c_str());
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

  int node_nums = 5;
  uint64_t node_times = 1;
  uint64_t value_times = 1;
  if (argc == 1) {
    LOG_INFO("Use default node_nums:%s5%s node_times:%s1%s value_times:%s1%s", 
             TXT_RED, NRM, TXT_RED, NRM, TXT_RED, NRM);
  }
  if (argc > 1)
    node_nums = atoi(argv[1]);
  if (argc > 2) {
    node_times = atoll(argv[2]);
    if (node_times > node_nums) {
      LOG_INFO("node_times %sMUST%s <= node_nums RETURN!", TXT_RED, NRM);
      return -1;
    }
  }
  if (argc > 3) 
    value_times = atoll(argv[3]);

  std::set<node_id_t> nodes;
  // init all nodes set
  for (int i = 0; i < node_nums; i++) 
    nodes.insert(i);

  std::vector<Captain *> captains;
  std::vector<View *> views;
  // init all view & captain
  for (int i = 0; i < node_nums; i++) {
    views.push_back(new View(i, nodes));
    captains.push_back(new Captain(*views[i]));
  }
  
  Commo commo(captains);
  pool tp(4);
  commo.set_pool(&tp);
  callback_t callback = do_sth;
  // set commo for every captain & init a new client thread
  for (int i = 0; i < node_nums; i++) {
    captains[i]->set_commo(&commo);
    captains[i]->set_callback(callback);
  }

  for (int i = 0; i < node_times; i++) {
    for (int j = 0; j < value_times; j++) {
      LOG_INFO("***********************************************************************");
      std::string value = "Love MS Time_" + std::to_string(j) + " from Node_" + std::to_string(i);
      LOG_INFO("** Commit Value--[%s] Start", value.c_str());
      captains[i]->commit_value(value);
//      client_commit_file(captains[i]);
      LOG_INFO("** (Client):%d (Commit_Times):%d END", i, j);
      LOG_INFO("***********************************************************************");
    }
  }

  tp.wait();
  LOG_INFO("** END **");
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
