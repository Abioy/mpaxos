/*
 * proposer.hpp
 *    Author: Lijing Wang
 */

#pragma once

#include "mpaxos.pb.h"
//#include <cstdint>
#include <set>
namespace mpaxos {

//typedef uint16_t node_id_t;
//typedef uint64_t slot_id_t;
//typedef uint64_t ballot_id_t;
//typedef uint64_t value_id_t;

using node_id_t = uint16_t;
using slot_id_t = uint64_t;
using ballot_id_t = uint64_t;
using value_id_t = uint64_t;

class View {
 public:
//  View(set<node_id_t> &nodes);
  View();
  std::set<node_id_t> * get_nodes();
  std::set<node_id_t> nodes_;
};

class Proposer {
 public:
  Proposer();
//  Proposer(View &view, PropValue &value, std::function<void>(PropValue &value) &callback); 
  Proposer(View &view, PropValue &value); 
  ~Proposer();

  // start proposing, send prepare requests to at least a majority of acceptors.
  void msg_prepare();

  // choose a higher ballot id, retry the prepare phase.
  // clean the map for the ack of prepare and accept
  void restart_msg_prepare();

  // start accepting, send accept requests to acceptors who responsed.
  void msg_accept();

  /**
   * handle acks to the prepare requests.
   * if a majority of yes, decide use which value to do the accept requests.
   *   with a majority of empty, use the initial value I have. 
   *   with some already accepted value, choose one with the highest ballot
   * if a majority of no, 
   * restart with a higher ballot 
   */ 
  void handle_msg_promise(MsgAckPrepare *);

  /**
   * handle acks to the accept reqeusts;
   * if a majority of yes, the value is successfully chosen. 
   * if a majority of no, restart the first phase with a higher ballot.
   */
  void handle_msg_accepted(MsgAckAccept *);   

  /**
   * ballot_id_t should be a 64-bit uint, high 48bit is self incremental counter,
   * low 16bit is the node id.
   */ 
  ballot_id_t gen_next_ballot() ;

  View *view_; 

  // the value that client wants me to commit
  PropValue *init_value_; 

  // the ballot I am currently using
  ballot_id_t curr_ballot_; 
  
  // the value I am currently proposing
  PropValue *curr_value_;  
  
  // the max ballot I have ever seen, initialy 0;
  ballot_id_t max_ballot_;  

  // the max PropValue I have ever seen, initialy null.
  PropValue *max_value_;

  // the callback after I finish this instance.  
//  std::function<void>(PropValue &value) callback_;

  // the acks from all nodes to the current prepare.  
  // remember to clean for every next ballot!!!
  std::map<node_id_t, MsgAckPrepare *> msg_ack_prepare_;

  // the ack from all nodes to the cuurent accept. 
  // remember to clean for every next ballot!!!
  std::map<node_id_t, MsgAckAccept *> msg_ack_accept_;
};
}  // namespace mpaxos
