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
  auto t1 = std::chrono::high_resolution_clock::now();
  LOG_INFO("** START **");

  int node_nums = 5;
  int total_times = 1;

  if (argc == 1) {
    std::cout << "Use default node_nums:5 total_times:1(only Node_0 will commit one time)\n you can specify the node_nums and the total_times of all nodes" << std::endl;
    std::cout << "Every time before commit_value (randomly), one node will crash or start" << std::endl;
  }
  if (argc > 1)
    node_nums = atoi(argv[1]);
  if (argc > 2)
    total_times = atoi(argv[2]);

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
  std::vector<std::thread *> clients; 
  // set commo for every captain & init a new client thread
  for (int i = 0; i < node_nums; i++) {
    captains[i]->set_commo(&commo);
//    client_commit(captains[i]);
//    clients.push_back(new std::thread(client_commit, captains[i]));
  }

  std::vector<int> node_times_vec(node_nums, 0); 

  int node_id = 0;
  for (int i = 0; i < total_times; i++) {

    auto t2 = std::chrono::high_resolution_clock::now();
    srand(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    int chosen_id = rand() % node_nums;
    if (captains[chosen_id]->get_status()) {
      captains[chosen_id]->crash();
      LOG_INFO("%s** Node_%d Crash @ Total_Time_%d%s", BAK_RED, chosen_id, i, NRM);
    } else {
      captains[chosen_id]->recover();
      LOG_INFO("%s** Node_%d Recover @ Total_Time_%d%s", BAK_GRN, chosen_id, i, NRM);
    }
    
    int alive_count = 0;
    for (int j = 0; j < node_nums; j++) {
      if (captains[j]->get_status())
        alive_count++;
    }
    LOG_INFO("%s** %d Node/Nodes is/are Alive...%s", BAK_BLU, alive_count, NRM);

    if (alive_count == 0) {
      // randomly pick a node to recover
      t2 = std::chrono::high_resolution_clock::now();
      srand(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
      int chosen_id = rand() % node_nums;
      captains[chosen_id]->recover();
      LOG_INFO("%s** Because No Node is Alive, Pick up Node_%d to Recover~~%s", BAK_GRN, chosen_id, NRM);
    }

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

    node_times_vec[node_id]++;
  }

  LOG_INFO("** END **");
  return EXIT_SUCCESS;
}
} // namespace mpaxos

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
