/**
 * captain.cpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */

#include "captain.hpp"
#include "commo.hpp"
#include <iostream>
namespace mpaxos {

Captain::Captain(View &view)
  : view_(&view), max_chosen_(0), curr_proposer_(NULL), commo_(NULL) {
  curr_value_ = new PropValue();
  curr_value_->set_id(view_->whoami());
}

Captain::~Captain() {
}

/** 
 * return node_id
 */
node_id_t Captain::get_node_id() {
  return view_->whoami();
}
/**
 * set commo_handler 
 */
void Captain::set_commo(Commo *commo) {
  commo_ = commo;
}
/** 
 * client commits one value to captain
 */
void Captain::commit_value(std::string data) {
//  std::lock_guard<std::mutex> lock(mutex_);
  std::cout << "Captain in commit_value" << std::endl;
  if (!tocommit_values_.empty() || curr_proposer_) {
    tocommit_values_.push(data);
    return;
  } 
  // prepare value
  std::cout << "Captain Prepare Value" << std::endl;
  curr_value_->set_data(data);
  value_id_t value_id = curr_value_->id() + (1 << 16) + view_->whoami();
  curr_value_->set_id(value_id);
  std::cout << "curr_value id: " << curr_value_->id() 
            << " value: " << curr_value_->data() << std::endl; 
  // start a new instance
  new_slot();
  std::cout << "Captain commit_value Over" << std::endl;
}
/**
 * captain starts phaseI
 */
void Captain::new_slot() {
  // new proposer
  std::cout << "Captain in new_slot" << std::endl;
  curr_proposer_ = new Proposer(*view_, *curr_value_);
  // new acceptor
  std::cout << "WHOOOO " << view_->whoami() << std::endl;
  // IMPORTANT!!! Only when there is no such slot acceptor, init
  if (acceptors_.count(max_chosen_ + 1) == 0)
    acceptors_[max_chosen_ + 1] = new Acceptor(*view_);
  std::cout << "slot_id (max_chosen_ + 1)" << max_chosen_ + 1 << std::endl;
  // start phaseI
  std::cout << "Captain new_slot : before call msg_prepare" << std::endl;
  MsgPrepare *msg_pre = curr_proposer_->msg_prepare();
  // important!! captain set slot_id
  msg_pre->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
  commo_->broadcast_msg(msg_pre, PREPARE);
  std::cout << "Captain new_slot : after broadcast_msg Over" << std::endl;
}
/**
 * handle message from commo, all kinds of message
 */
void Captain::handle_msg(google::protobuf::Message *msg, MsgType msg_type) {
  std::cout << "Captain handle_msg Start " << std::endl;
//  std::lock_guard<std::mutex> lock(mutex_);
  switch (msg_type) {
    case PREPARE: {
      // acceptor should handle prepare message
      MsgPrepare *msg_pre = (MsgPrepare *)msg;
      slot_id_t acc_slot = msg_pre->msg_header().slot_id();
      std::cout << "PREPARE slot_id " << acc_slot << std::endl;
      // IMPORTANT!!! if there is no such acceptor then init
      if (acceptors_.count(acc_slot) == 0) 
        acceptors_[acc_slot] = new Acceptor(*view_);
      MsgAckPrepare * msg_ack_pre = acceptors_[acc_slot]->handle_msg_prepare(msg_pre);
      commo_->send_one_msg(msg_ack_pre, PROMISE, msg_pre->msg_header().node_id());
      break;
    }
    case PROMISE: {
      // proposer should handle ack of prepare message
      // IMPORTANT! if curr_proposer_ == NULL Drop TODO can send other info
      if (!curr_proposer_) return;
      MsgAckPrepare *msg_ack_pre = (MsgAckPrepare *)msg;
      switch (curr_proposer_->handle_msg_promise(msg_ack_pre)) {
        case DROP: break;
        case NOT_ENOUGH: break;
        case CONTINUE: {
          // Send to all acceptors in view
          std::cout << "Continue to Phase II" << std::endl;
          MsgAccept *msg_acc = curr_proposer_->msg_accept();
          msg_acc->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
          commo_->broadcast_msg(msg_acc, ACCEPT);
          break;
        }
        case RESTART: {  //RESTART
          MsgPrepare *msg_pre = curr_proposer_->restart_msg_prepare();
          msg_pre->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
          commo_->broadcast_msg(msg_pre, PREPARE);
        }
        default: {
        }
      }
      break;
    }
    case ACCEPT: {
      // acceptor should handle accept message
      MsgAccept *msg_acc = (MsgAccept *)msg;
      slot_id_t acc_slot = msg_acc->msg_header().slot_id();
      std::cout << "ACCEPT slot_id " << acc_slot << std::endl;
      // IMPORTANT!!! if there is no such acceptor then init
      if (acceptors_.count(acc_slot) == 0) 
        acceptors_[acc_slot] = new Acceptor(*view_);
      MsgAckAccept *msg_ack_acc = acceptors_[acc_slot]->handle_msg_accept(msg_acc);
      commo_->send_one_msg(msg_ack_acc, ACCEPTED, msg_acc->msg_header().node_id());
      break;
    } 
    case ACCEPTED: {
      // proposer should handle ack of accept message
      // IMPORTANT! if curr_proposer_ == NULL Drop TODO can send other info
      if (!curr_proposer_) return;
      MsgAckAccept *msg_ack_acc = (MsgAckAccept *)msg; 
      // handle_msg_accepted
      switch (curr_proposer_->handle_msg_accepted(msg_ack_acc)) {
        case DROP: break;
        case NOT_ENOUGH: break;
        case CHOOSE: {
          std::cout << "One Value is Successfully Chosen!" << std::endl;
          // First add the chosen_value into chosen_values_ 
          PropValue *chosen_value = curr_proposer_->get_chosen_value();
          // IMPORTANT 
          chosen_values_[max_chosen_ + 1] = chosen_value;
          // self increase max_chosen_
          max_chosen_++;
          if (chosen_value->id() == curr_value_->id()) {
            // client's commit succeeded, if no value to commit, set NULL
            if (tocommit_values_.empty()) {
              std::cout << "Proposer END MISSION" << std::endl;
              curr_proposer_ = NULL;
              return;
            }
            // start committing a new value from queue
            curr_value_->set_data(tocommit_values_.front());
            // pop the value
            tocommit_values_.pop();
            value_id_t value_id = curr_value_->id() + (1 << 16) + view_->whoami();
            curr_value_->set_id(value_id);
            // delete curr_proposer_
            delete curr_proposer_;
            // start a new slot
            new_slot();
          } else {
            // recommit the same value
            delete curr_proposer_;
            new_slot();
          }
          break;
        }
        default: { //RESTART
          MsgPrepare *msg_pre = curr_proposer_->restart_msg_prepare();
          msg_pre->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
          commo_->broadcast_msg(msg_pre, PREPARE);
        }
      }
      break;
    }
    default: 
      break;
  }
}
} //  namespace mpaxos
