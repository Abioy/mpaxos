/**
 * test_captain.cpp
 * To test Proposer Acceptor Captain Commo(No Message exchange) 
 * Author: Lijing Wang
 */

//#include "commo.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include "detect_error.hpp"

namespace mpaxos {

void do_sth(slot_id_t slot_id, std::string& data) {
//  LOG_INFO("HAHA slot_id:%llu value:%s", slot_id, data.c_str());
}

static void sig_int(int num) {
  return;
}

int main(int argc, char** argv) {
  LOG_INFO("** START **");

  signal(SIGINT, sig_int);

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
//  pool tp(node_nums * 2);
  pool tp(1);

  // init all view & captains 
  for (int i = 0; i < node_nums; i++) {
    views.push_back(new View(i, nodes));
    captains.push_back(new Captain(*views[i], &tp));
  }
  
  callback_t callback = do_sth;
  // set commo for every captain & init a new client thread
  for (int i = 0; i < node_nums; i++) {
    captains[i]->set_callback(callback);
  }

  int total = 0;
  for (int i = 0; i < node_times; i++) {
    for (int j = 0; j < value_times; j++) {
      LOG_DEBUG("***********************************************************************");
      std::string value = "Love MS Time_" + std::to_string(j) + " from Node_" + std::to_string(i) + " Total_Time_" + std::to_string(total);
      LOG_INFO("** Commit Value--[%s] Start", value.c_str());
      captains[i]->commit_value(value);
//      client_commit_file(captains[i]);
      LOG_DEBUG("** (Client):%d (Commit_Times):%d END", i, j);
      LOG_DEBUG("***********************************************************************");
      total++;
    }
  }

//  LOG_INFO("HEHE");
//  boost::this_thread::sleep_for(boost::chrono::seconds(4));
  tp.wait();
  LOG_INFO("Start sleeping!");
  sleep(10);
//  std::vector<std::vector<PropValue *> > results;
  int total_times = node_times * value_times;
  LOG_INFO("BEFORE TEST!!! (total_times):%d", total_times);
  Detection det(captains, total_times);
  if (!det.detect_all()) 
    det.print_one();

//  captains[0]->print_chosen_values();
  LOG_INFO("** DONE! **");
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
