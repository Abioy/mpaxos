/**
 * proposer.cpp
 * Created on April 27, 2015
 * Author: Lijing Wang
 */

#include "proposer.hpp"

namespace mpaxos {

  View::View() {
    nodes_.insert(0);
  }

  std::set<node_id_t> * View::get_nodes() {
    return &nodes_;
  }

  Proposer::Proposer(View &view, PropValue &value) 
//                     std::function<void>(PropValue &value) &callback) 
    : view_(&view), init_value_(&value),// callback_(callback),
      curr_ballot_(0), curr_value_(new PropValue(value)),
      max_ballot_(0), max_value_(NULL) {
    // init msg_ack_prepare_ & msg_ack_accept_ according to nodes_set
    std::set<node_id_t>::iterator it;
    std::set<node_id_t> * nodes_set = view_->get_nodes();
    for (it = nodes_set->begin(); it != nodes_set->end(); it++) {
      msg_ack_prepare_[*it] = NULL;
      msg_ack_accept_[*it] = NULL;
    }
    // init id of init_value_
    init_value_->set_id(0);
    // init id of curr_value_
    curr_value_->set_id(0);

  }

  Proposer::Proposer() : max_ballot_(0), max_value_(NULL) {
  }

  Proposer::~Proposer() {
  }

  // start proposing, send prepare requests to at least a majority of acceptors.
  void Proposer::msg_prepare() {
  }

  // choose a higher ballot id, retry the prepare phase.
  // clean the map for the ack of prepare and accept
  void Proposer::restart_msg_prepare() {
  }

  // start accepting, send accept requests to acceptors who responsed.
  void Proposer::msg_accept() {
  }

  /**
   * handle acks to the prepare requests.
   * if a majority of yes, decide use which value to do the accept requests.
   *   with a majority of empty, use the initial value I have. 
   *   with some already accepted value, choose one with the highest ballot
   * if a majority of no, 
   * restart with a higher ballot 
   */ 
  void Proposer::handle_msg_promise(MsgAckPrepare *) {
  }

  /**
   * handle acks to the accept reqeusts;
   * if a majority of yes, the value is successfully chosen. 
   * if a majority of no, restart the first phase with a higher ballot.
   */
  void Proposer::handle_msg_accepted(MsgAckAccept *) {
  }   

  /**
   * ballot_id_t should be a 64-bit uint, high 48bit is self incremental counter,
   * low 16bit is the node id.
   */ 
  ballot_id_t Proposer::gen_next_ballot() {
    ballot_id_t next_ballot_id;
    return next_ballot_id;
  }
}
