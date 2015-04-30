/**
 * proposer.cpp
 * Created on April 27, 2015
 * Author: Lijing Wang
 */

#include "proposer.hpp"

namespace mpaxos {

  View::View(node_id_t node_id) : node_id_(node_id) {
    nodes_.insert(0);
  }

  std::set<node_id_t> * View::get_nodes() {
    return &nodes_;
  }

  node_id_t View::whoami() {
    return node_id_;
  }

  Proposer::Proposer(View &view, PropValue &value) 
//                     std::function<void>(PropValue &value) &callback) 
    : view_(&view), init_value_(&value),// callback_(callback),
      curr_ballot_(0), curr_value_(new PropValue(value)),
      max_ballot_(0), max_value_(NULL) {
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
  // Now just send to all acceptors according to ack_map;
  MsgPrepare *Proposer::msg_prepare() {
    MsgPrepare *msg_pre = new MsgPrepare();
    MsgHeader *msg_header = new MsgHeader();
    msg_header->set_msg_type(MsgType::PREPARE);
    msg_header->set_node_id(view_->whoami());
    msg_header->set_slot_id(0);
    // prepare the msg_prepare
    msg_pre->set_allocated_msg_header(msg_header);
    // gen_next_ballot will increase the curr_value_
    msg_pre->set_ballot_id(gen_next_ballot());
    return msg_pre;
  }

  // choose a higher ballot id, retry the prepare phase.
  // clean the map for the ack of prepare and accept
  MsgPrepare *Proposer::restart_msg_prepare() {
    curr_value_ = NULL;
    // clear map
    msg_ack_prepare_.clear();
    msg_ack_accept_.clear();
    return msg_prepare();
  }

  // start accepting, send accept requests to acceptors who responsed.
  MsgAccept *Proposer::msg_accept() {
    MsgAccept *msg_acc = new MsgAccept();
    MsgHeader *msg_header = new MsgHeader();
    msg_header->set_msg_type(MsgType::ACCEPT);
    msg_header->set_node_id(view_->whoami());
    msg_header->set_slot_id(0);
    msg_acc->set_allocated_msg_header(msg_header);
    msg_acc->set_ballot_id(curr_ballot_);
    msg_acc->set_allocated_prop_value(curr_value_);
    return msg_acc;      
  }

//  // Temp return msg_ack_prepare_ to captain
//  std::map<node_id_t, MsgAckPrepare *> *Proposer::get_ack_prepare() {
//    return msg_ack_prepare_;
//  }

//  // Temp return msg_ack_accept_ to captain
//  std::map<node_id_t, MsgAckAccept *> *Proposer::get_ack_accept() {
//    return msg_ack_accept_;
//  }

  // Temp return curr_value_ to captain
  PropValue *Proposer::get_curr_value() {
    return curr_value_;
  }

  /**
   * handle acks to the prepare requests.
   * if a majority of yes, decide use which value to do the accept requests.
   *   with a majority of empty, use the initial value I have. 
   *   with some already accepted value, choose one with the highest ballot
   * if a majority of no, 
   * restart with a higher ballot 
   */ 
  int Proposer::handle_msg_promise(MsgAckPrepare *msg_ack_pre) {
    std::lock_guard<std::mutex> lock(prepare_mutex_);
    //DROP Out of Date & Already enter Phase II
    if (msg_ack_pre->ballot_id() < curr_ballot_ || curr_value_) return DROP; 
    node_id_t node_id = (uint16_t)msg_ack_pre->msg_header().node_id();
    std::cout << "Inside handle_msg_promise node_id: " << node_id << std::endl;
    msg_ack_prepare_[node_id] = msg_ack_pre;
    // NOT_ENOUGH
    if (msg_ack_prepare_.size() <= view_->get_nodes()->size() / 2) 
      return NOT_ENOUGH; 
    int true_counter = 0;
    std::map<node_id_t, MsgAckPrepare *>::iterator it;
    for (it = msg_ack_prepare_.begin(); it != msg_ack_prepare_.end(); it++) {
      if (it->second->reply()) {
        if (it->second->max_ballot_id() > max_ballot_) {
          max_ballot_ = it->second->max_ballot_id(); 
          max_value_ = it->second->mutable_max_prop_value();
        }
        true_counter++;
      }
    }
    if (true_counter > view_->get_nodes()->size() / 2) {
      // CONTINUE
      curr_value_ = max_value_ == NULL ? init_value_ : max_value_;
      return CONTINUE;
    } else // RESTART 
      return RESTART;
  }

  /**
   * handle acks to the accept reqeusts;
   * if a majority of yes, the value is successfully chosen. 
   * if a majority of no, restart the first phase with a higher ballot.
   */
  int Proposer::handle_msg_accepted(MsgAckAccept *msg_ack_acc) {
    std::lock_guard<std::mutex> lock(accept_mutex_);
    //Drop Out of Date or Out of Order ACK
    if (msg_ack_acc->ballot_id() < curr_ballot_ || !curr_value_) return DROP;
    node_id_t node_id = (uint16_t)msg_ack_acc->msg_header().node_id();
    std::cout << "Inside handle_msg_accepted node_id: " << node_id << std::endl;
    msg_ack_accept_[node_id] = msg_ack_acc;
    // NOT_ENOUGH
    if (msg_ack_accept_.size() <= view_->get_nodes()->size() / 2) 
      return NOT_ENOUGH; 
    int true_counter = 0;
    std::map<node_id_t, MsgAckAccept *>::iterator it;
    for (it = msg_ack_accept_.begin(); it != msg_ack_accept_.end(); it++) {
      if (it->second->reply()) {
        true_counter++;
      }
    }
    if (true_counter > view_->get_nodes()->size() / 2) {
      // ACCEPT 
      return CHOOSE;
    } else // RESTART 
      return RESTART;    
  }   

  /**
   * ballot_id_t should be a 64-bit uint, high 48bit is self incremental counter,
   * low 16bit is the node id.
   */ 
  ballot_id_t Proposer::gen_next_ballot() {
    curr_ballot_ = (curr_ballot_ >> 16 + 1 ) << 16 + view_->whoami();
    return curr_ballot_;
  }
}