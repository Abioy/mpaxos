/**
 * captain.hpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */

#pragma once
#include "proposer.hpp"
#include "acceptor.hpp"
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
   * set commo_handler 
   */
  void set_commo(Commo *); 
  /**
   * handle message from commo, all kinds of message
   */
  void handle_msg(google::protobuf::Message *, MsgType);

 private:
  View *view_;
  std::map<slot_id_t, Acceptor *> acceptors_;
  std::map<slot_id_t, PropValue *> chosen_values_;
  std::queue<std::string> tocommit_values_;
  // max chosen instance/slot id 
  slot_id_t max_chosen_; 
  PropValue *curr_value_;  
  Proposer *curr_proposer_;
  Commo *commo_;
  std::mutex mutex_;
};
} //  namespace mpaxos
