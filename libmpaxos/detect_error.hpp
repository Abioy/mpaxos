/**
 * detect_error.hpp
 * Author: Lijing Wang
 * Date: 5/26/2015
 */

#pragma once

#include "captain.hpp"
namespace mpaxos {

class Detection {
 public:
  Detection(std::vector<Captain *> &captains, uint64_t total_times) 
    : captains_(&captains), total_times_(total_times), node_id_(0) {
    node_nums_ = captains.size();
    for (int i = 0; i < node_nums_; i++) {
      results_.push_back(captains[i]->get_chosen_values());
    } 
  }
//  ~Detection();

  bool detect_size() {
    bool ret = true;
    for (int i = 0; i < node_nums_; i++) {
      if (results_[i].size() != total_times_ + 1) {
        LOG_INFO("Result ERROR Size not EQUAL! (NodeID): %d (chosen_values.size): %lu (total_times): %llu", i, results_[i].size(), total_times_);
        ret = false;
      }
    } 
    return ret;
  }
  
  bool detect_content() {
    bool ret = true;
    for (int i = 1; i <= total_times_; i++) {
      int k = 0;
      while (results_[k][i] == NULL && k < node_nums_) 
        k++;  
      if (k == node_nums_) continue;

      for (int j = 1; j < node_nums_; j++) {
        if (results_[j][i] && results_[j][i]->id() != results_[k][i]->id()) {
          LOG_INFO("Value Not EQUAL! slot_id: %d", i);
          LOG_INFO("NodeID_%d %s", j, results_[j][i]->data().c_str());
          LOG_INFO("NodeID_%d %s", k, results_[k][i]->data().c_str());
          node_id_ = j;
          ret = false;
        }
      }
    }
    return ret;
  }

  bool detect_unique() {
    bool ret = true;
    for (int i = 1; i <= total_times_; i++) {
      for (int j = i + 1; j <= total_times_; j++) {
        if (results_[0][i]->id() == results_[0][j]->id()) {
          LOG_INFO("Value Not UNIQUE! slot_id: %d & slot_id: %d", i, j);
          LOG_INFO("Value: %s", results_[0][j]->data().c_str());
          ret = false;
        }
      }
    }
   return ret; 
  }

  bool detect_unique_all() {
    bool ret = true;
    for (int k = 0; k < node_nums_; k++) {
      int size = results_[k].size();
      for (int i = 1; i < size; i++) {
        if (results_[k][i] == NULL) continue;
        for (int j = i + 1; j < size; j++) {
          if (results_[k][j] && results_[k][i]->id() == results_[k][j]->id()) {
            LOG_INFO("Value Not UNIQUE! slot_id: %d & slot_id: %d", i, j);
            LOG_INFO("Value: %s", results_[k][j]->data().c_str());
            ret = false;
          }
        }
      }
    }
    return ret; 
  }

  bool detect_all() {
    bool ret = true;
    if (detect_size()) {
      LOG_INFO("%sSIZE TEST PASS%s", BLD_GRN, NRM);
      if (detect_content()) {
        LOG_INFO("%sCONTENT TEST PASS%s", BLD_GRN, NRM);
        if (detect_unique()) {
          LOG_INFO("%sUNIQUE TEST PASS%s", BLD_GRN, NRM);
          LOG_INFO("%sALL PASS ^_^%s", BLD_GRN, NRM);
        } else {
          LOG_INFO("%sERROR! UNIQUE!%s", BLD_RED, NRM);
          ret = false;
        }
      } else {
        LOG_INFO("%sERROR! CONTENT!%s", BLD_RED, NRM);
        ret = false;
      }
    } else {
      LOG_INFO("%sERROR! SIZE!%s", BLD_RED, NRM);
      ret = false;
    }
    return ret;
  }

  bool detect_down() {
    bool ret = true;
    if (detect_content()) {
      LOG_INFO("%sCONTENT TEST PASS%s", BLD_GRN, NRM);
      if (detect_unique()) {
        LOG_INFO("%sUNIQUE TEST PASS%s", BLD_GRN, NRM);
        LOG_INFO("%sALL PASS ^_^%s", BLD_GRN, NRM);
      } else {
        LOG_INFO("%sERROR! UNIQUE!%s", BLD_RED, NRM);
        ret = false;
      }
    } else {
      LOG_INFO("%sERROR! CONTENT!%s", BLD_RED, NRM);
      ret = false;
    }
    return ret;
  }

  void print_all() {
    for (int i = 0; i < node_nums_; i++) 
      captains_->at(i)->print_chosen_values();
  }

  void print_one() {
    captains_->at(node_id_)->print_chosen_values();
  }
 
 private:
  std::vector<Captain *> *captains_;
  std::vector<std::vector<PropValue *> > results_;
  int node_nums_;
  uint64_t total_times_;
  int node_id_;
};

} // namespace
