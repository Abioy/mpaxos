/**
 * captain.hpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */

#pragma once
#include "proposer.hpp"
#include "acceptor.hpp"
#include <queue>
#include <unordered_map>
//#include "include_all.h"
namespace mpaxos {
// TODO [Loli] This module is responsible for locating the correct proposer and
// acceptor for a certain instance (an instance is identified by slot_id). 
class Commo;
class Captain {
 public:
  Captain(View &);
  ~Captain();

  /** 
   * client commits one value to captain
   */
  void commit_value(std::string);

  /**
   * captain starts a new paxos instance 
   */
  void new_slot();

  /** 
   * return node_id
   */
  node_id_t get_node_id(); 

  /**
   * set commo_handler 
   */
  void set_commo(Commo *); 

  /**
   * handle message from commo, all kinds of message
   */
  void handle_msg(google::protobuf::Message *, MsgType);

  /** 
   * Callback function after commit_value  
   */
  void clean();

  void crash();

  void recover();

  bool get_status();

 private:

  View *view_;
//  std::map<slot_id_t, Acceptor *> acceptors_;
  std::vector<Acceptor *> acceptors_;
//  std::unordered_map<slot_id_t, PropValue *> chosen_values_;
  std::vector<PropValue *> chosen_values_;
  std::queue<std::string> tocommit_values_;

  // max chosen instance/slot id 
  slot_id_t max_chosen_; 
  // don't empty this, need value_id to do self increment
  PropValue *curr_value_;  
  Proposer *curr_proposer_;
  Commo *commo_;
  std::mutex mutex_;
  // tag work 
  bool work_;

};

} //  namespace mpaxos
