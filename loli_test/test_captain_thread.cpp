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

int main(int argc, char** argv) {
  LOG_INFO("** START **");

//  ThreadPool pool(4);

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
//    captains[i]->set_thread_pool(&pool);
  }
  
  Commo commo(captains);
  pool tp(4);
  commo.set_pool(&tp);
  // set commo for every captain & init a new client thread
  for (int i = 0; i < node_nums; i++) {
    captains[i]->set_commo(&commo);
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

//  LOG_INFO("HEHE");
//  boost::this_thread::sleep_for(boost::chrono::seconds(4));
  tp.wait();
  std::vector<std::vector<PropValue *> > results;
  int total_times = node_times * value_times;
  for (int i = 0; i < node_nums; i++) {
//    LOG_INFO("NodeID %d (chosen_values_):", i);
//    captains[i]->print_chosen_values();
    results.push_back(captains[i]->get_chosen_values());
    if (results[i].size() != total_times + 1) {
      LOG_INFO("Result ERROR Size not EQUAL! (NodeID): %d (chosen_values.size): %lu (total_times): %d", i, results[i].size(), total_times);
      return -1;
    }
  } 


  for (int i = 1; i <= total_times; i++) {
    if (results[0][i] == NULL) continue;
    for (int j = 1; j < node_nums; j++) {
      if (results[j][i] && results[j][i]->id() != results[0][i]->id()) {
        LOG_INFO("Value Not EQUAL! slot_id: %d", i);
      }
    }
  }

  captains[0]->print_chosen_values();
  LOG_INFO("** DONE! **");
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
