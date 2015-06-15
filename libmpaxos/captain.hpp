/**
 * captain.hpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */

#pragma once
#include "proposer.hpp"
//#include "commo.hpp"
#include "acceptor.hpp"
#include <queue>
#include <unordered_map>
#include "threadpool.hpp" 
#include "ThreadPool.h"
//#include "include_all.h"
using namespace boost::threadpool;
namespace mpaxos {
// TODO [Loli] This module is responsible for locating the correct proposer and
// acceptor for a certain instance (an instance is identified by slot_id). 
class Commo;
class Captain {
 public:

  Captain(View &);
  Captain(View &, pool *);
  ~Captain();

  /**
   * set_callback from outside
   */
  void set_callback(callback_t& cb);

  /**
   * set_callback from outside
   */
  void set_callback(callback_full_t& cb);

  /**
   * TODO for loli NO USE 
   */
  void commit(std::string&);

  /** 
   * client commits one value to captain
   */
  void commit_value(std::string);

  /** 
  * client commits recover value to captain
  */
  void commit_recover();

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
   * set set thread_pool handler 
   */
//  void set_thread_pool(ThreadPool *); 

  /**
   * handle message from commo, all kinds of message
   */
  void handle_msg(google::protobuf::Message *, MsgType);

  /**
   * Add a new chosen_value 
   */
  void add_chosen_value(PropValue *);

  /**
   * Add a new learn_value 
   */
  void add_learn_value(slot_id_t, PropValue *, node_id_t);

  /**
   * Return Msg_header
   */
  MsgHeader *set_msg_header(MsgType);

  /**
   * Return Msg_header
   */
  MsgHeader *set_msg_header(MsgType, slot_id_t);

  /**
   * Return Decide Message
   */
  MsgDecide *msg_decide(slot_id_t);

  /**
   * Return Learn Message
   */
  MsgLearn *msg_learn(slot_id_t);

  /**
   * Return Teach Message
   */
  MsgTeach *msg_teach(slot_id_t);

  /** 
   * Callback function after commit_value  
   */
  void clean();

  void crash();

  void recover();

  bool get_status();

  void print_chosen_values();

  std::vector<PropValue *> get_chosen_values();

  bool if_recommit();

  void add_callback();

 private:

  View *view_;
//  std::map<slot_id_t, Acceptor *> acceptors_;
  std::vector<Acceptor *> acceptors_;
//  std::unordered_map<slot_id_t, PropValue *> chosen_values_;
  std::vector<PropValue *> chosen_values_;
  std::queue<std::string> tocommit_values_;

  // max chosen instance/slot id 
  slot_id_t max_chosen_; 
  // max_chosen_without_hole instance/slot id 
  slot_id_t max_chosen_without_hole_; 
  // callback_slot_
  slot_id_t callback_slot_;

  // don't empty this, need value_id to do self increment
  PropValue *curr_value_;  
  value_id_t value_id_;
  Proposer *curr_proposer_;
  // mark if value is chosen
  ProposerStatus proposer_status_;

  Commo *commo_;

  boost::mutex curr_value_mutex_;
  boost::mutex curr_proposer_mutex_;
//  boost::mutex chosen_values_mutex_;
  boost::mutex acceptors_mutex_;
  boost::mutex max_chosen_mutex_;
  boost::mutex max_chosen_without_hole_mutex_;
  boost::mutex work_mutex_;
  boost::mutex tocommit_values_mutex_;
  boost::mutex callback_mutex_;
  /** 
   * for loli, trigger this callback 
   * sequentially for each chosen value.
   */
  callback_t callback_;
  callback_full_t callback_full_;
  pool *pool_;

  // tag work 
  bool work_;

};

} //  namespace mpaxos
