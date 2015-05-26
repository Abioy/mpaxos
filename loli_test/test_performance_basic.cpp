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

  int node_nums = 1;
  uint64_t node_times = 1;
  uint64_t value_times = 100000;
  if (argc == 1) {
    LOG_INFO("Use default node_nums:%s1%s node_times:%s1%s value_times:%s100,000%s", 
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
  if (argc > 3) {
    value_times = atoll(argv[3]);
    if (value_times < 100) {
      LOG_INFO("Performance Test, please input %svalue_times >= 100%s RETURN!", TXT_RED, NRM);
      return -1;
    }
  }

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

  std::vector<std::thread *> clients; 
  // set commo for every captain & init a new client thread
  for (int i = 0; i < node_nums; i++) {
    captains[i]->set_commo(&commo);
//    client_commit(captains[i]);
//    clients.push_back(new std::thread(client_commit, captains[i]));
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < node_times; i++) {
    for (int j = 0; j < value_times; j++) {
//      LOG_INFO("***********************************************************************");
      std::string value = "Love MS Time_" + std::to_string(j) + " from Node_" + std::to_string(i);
//      LOG_INFO("** Commit Value--[%s] Start", value.c_str());
      captains[i]->commit_value(value);
//      client_commit_file(captains[i]);
//      LOG_INFO("** (Client):%d (Commit_Times):%d END", i, j);
//      LOG_INFO("***********************************************************************");
    }
  }
  LOG_INFO("node_nums:%s%d%s node_times:%s%llu%s value_times:%s%llu%s", 
           TXT_RED, node_nums, NRM, TXT_RED, node_times, NRM, TXT_RED, value_times, NRM);
  auto t2 = std::chrono::high_resolution_clock::now();
  uint64_t time_count = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  LOG_INFO("Before wait ms:%llu throughput:%llu per second", time_count, value_times * node_times / time_count * 1000);
  tp.wait();
  t2 = std::chrono::high_resolution_clock::now();
  time_count = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  LOG_INFO("After wait ms:%llu throughput:%llu per second", time_count, value_times * node_times / time_count * 1000);
//  for (int i = node_nums -1 ; i >=0; i--) {
//    LOG_INFO("***********************************************************************");
//    LOG_INFO("** (Client):%d Commit Start", i);
//    client_commit(captains[i]);
//    LOG_INFO("** (Client):%d Commit END", i);
//    LOG_INFO("***********************************************************************");
//  }
//  std::this_thread::sleep_for(std::chrono::seconds(100));

//  for (int i = 0; i < node_nums; i++)
//    clients[i]->join();

  LOG_INFO("** END **");
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
