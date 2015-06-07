/**
 * test_captain.cpp
 * To test Proposer Acceptor Captain Commo(No Message exchange) 
 * Author: Lijing Wang
 */

#include "commo.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include "detect_error.hpp"
#include <assert.h>

namespace mpaxos {

void do_sth(slot_id_t slot_id, std::string& data) {
//  LOG_INFO("HAHA slot_id:%llu value:%s", slot_id, data.c_str());
}

int main(int argc, char** argv) {
  auto t1 = std::chrono::high_resolution_clock::now();
  LOG_INFO("** START **");

  int node_nums = 5;
  uint64_t total_times = 1;

  if (argc == 1) {
    LOG_INFO("Use default node_nums:%s5%s total_times:%s1%s", TXT_RED, NRM, TXT_RED, NRM);
    LOG_INFO("Every time before commit_value (randomly), one node will crash or start");
  }

  if (argc > 1)
    node_nums = atoi(argv[1]);
  if (argc > 2) 
    total_times = atoll(argv[2]);

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
  pool tp(1);
  commo.set_pool(&tp);

  callback_t callback = do_sth;
  // set commo for every captain & init a new client thread
  for (int i = 0; i < node_nums; i++) {
    captains[i]->set_commo(&commo);
    captains[i]->set_callback(callback);
  }

  std::vector<int> node_times_vec(node_nums, 0); 

  int node_id = 0;
  int alive_times = 0;
  int alive_count = node_nums;
  for (int i = 0; i < total_times; i++) {

    auto t2 = std::chrono::high_resolution_clock::now();
    srand(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    int chosen_id = rand() % node_nums;
    if (captains[chosen_id]->get_status()) {
      if (alive_count <= node_nums / 2 + 1) {
        LOG_INFO("%sNo Node Changes @ Total_Time_%d%s", BAK_MAG, i, NRM);
      } else {
        captains[chosen_id]->crash();
        LOG_INFO("%s** Node_%d Crash @ Total_Time_%d%s", BAK_RED, chosen_id, i, NRM);
      }
    } else {
      captains[chosen_id]->recover();
      LOG_INFO("%s** Node_%d Recover @ Total_Time_%d%s", BAK_GRN, chosen_id, i, NRM);
//      captains[chosen_id]->commit_value("RECOVER");
    }
    
    alive_count = 0;
    for (int j = 0; j < node_nums; j++) {
      if (captains[j]->get_status())
        alive_count++;
    }
    LOG_INFO("%s** %d Node/Nodes is/are Alive...%s", BAK_BLU, alive_count, NRM);

    while (1) {
      t2 = std::chrono::high_resolution_clock::now();
      srand(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
      node_id = rand() % node_nums;
      if (captains[node_id]->get_status())
        break;
    }

//    LOG_INFO("** Total_Time_%d Node_%d Times_:%d", i, node_id, node_times_vec[node_id]);
    std::string value = "Love MS Time_" + std::to_string(node_times_vec[node_id]) + " from Node_" + std::to_string(node_id) + " Total_Time_" + std::to_string(i);
    LOG_INFO("** %s", value.c_str());
    captains[node_id]->commit_value(value);
    LOG_INFO("** Node_%d Time_%d Total_Time_%d END", node_id, node_times_vec[node_id], i);
    LOG_INFO("***********************************************************************");

    alive_times++;
    node_times_vec[node_id]++;
  }

  tp.wait();
  LOG_INFO("Alive_times: %d", alive_times);
  Detection det(captains, alive_times);


  det.print_all();
  LOG_INFO("Before test~");
  det.detect_down();
  LOG_INFO("** END **");
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
