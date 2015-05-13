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
//  std::cout << "\nCaptain commit_value" << std::endl;
  LOG_INFO_CAP("<commit_value> Start");
  if (!tocommit_values_.empty() || curr_proposer_) {
    tocommit_values_.push(data);
    return;
  } 
  // prepare value
//  std::cout << "Captain Prepare Value" << std::endl;
  curr_value_->set_data(data);
  value_id_t value_id = curr_value_->id() + (1 << 16) + view_->whoami();
  curr_value_->set_id(value_id);
  LOG_DEBUG_CAP("(curr_value) id:%llu data:%s", curr_value_->id(), curr_value_->data().c_str());
//  std::cout << "curr_value id: " << curr_value_->id() 
//            << " value: " << curr_value_->data() << std::endl; 
  // start a new instance
  new_slot();
  LOG_INFO_CAP("<commit_value> Over!");
}

/**
 * captain starts phaseI
 */
void Captain::new_slot() {
  // new proposer
  LOG_DEBUG_CAP("<new_slot> Start");
  curr_proposer_ = new Proposer(*view_, *curr_value_);
  // new acceptor
//  std::cout << "WHOOOO view_->whoami()" << view_->whoami() << std::endl;
  // IMPORTANT!!! Only when there is no such slot acceptor, init
  if (acceptors_.count(max_chosen_ + 1) == 0) {
    LOG_DEBUG_CAP("<new_slot> init new acceptor id:%llu", max_chosen_ + 1);
    acceptors_[max_chosen_ + 1] = new Acceptor(*view_);
  }
//  std::cout << "Captain slot_id (max_chosen_ + 1)" << max_chosen_ + 1 << std::endl;
  // start phaseI
//  std::cout << "Captain new_slot : before call msg_prepare" << std::endl;
  MsgPrepare *msg_pre = curr_proposer_->msg_prepare();
  // important!! captain set slot_id
  msg_pre->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
  LOG_DEBUG_CAP("<new_slot> call <broadcast_msg> with (msg_type):PREPARE");
  commo_->broadcast_msg(msg_pre, PREPARE);
  LOG_DEBUG_CAP("<new_slot> call <broadcast_msg> Over");
}

/**
 * handle message from commo, all kinds of message
 */
void Captain::handle_msg(google::protobuf::Message *msg, MsgType msg_type) {
  LOG_DEBUG_CAP("<handle_msg> Start (msg_type):%d", msg_type);
//  std::lock_guard<std::mutex> lock(mutex_);
  switch (msg_type) {

    case PREPARE: {
      // acceptor should handle prepare message
      MsgPrepare *msg_pre = (MsgPrepare *)msg;

      slot_id_t acc_slot = msg_pre->msg_header().slot_id();
      LOG_DEBUG_CAP("(msg_type):PREPARE, (slot_id): %llu", acc_slot);
      // IMPORTANT!!! if there is no such acceptor then init
      if (acceptors_.count(acc_slot) == 0) {
        LOG_DEBUG_CAP("(msg_type):PREPARE, New Acceptor");
        acceptors_[acc_slot] = new Acceptor(*view_);
      }

      MsgAckPrepare * msg_ack_pre = acceptors_[acc_slot]->handle_msg_prepare(msg_pre);
      commo_->send_one_msg(msg_ack_pre, PROMISE, msg_pre->msg_header().node_id());
      break;
    }

    case PROMISE: {
      // proposer should handle ack of prepare message
      // IMPORTANT! if curr_proposer_ == NULL Drop TODO can send other info
      if (!curr_proposer_) {
        LOG_DEBUG_CAP("(msg_type):PROMISE, Value has been chosen and Proposer is NULL now! Return!");
        return;
      }
      MsgAckPrepare *msg_ack_pre = (MsgAckPrepare *)msg;

      if (msg_ack_pre->msg_header().slot_id() != max_chosen_ + 1) {
        LOG_DEBUG_CAP("(msg_type):PROMISE, This (slot_id):%llu is not (current_id):%llu! Return!", msg_ack_pre->msg_header().slot_id(), max_chosen_ + 1);
        return;
      }

      switch (curr_proposer_->handle_msg_promise(msg_ack_pre)) {
        case DROP: break;
        case NOT_ENOUGH: break;
        case CONTINUE: {
          // Send to all acceptors in view
          LOG_DEBUG_CAP("(msg_type):PROMISE, Continue to Phase II");
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

      LOG_DEBUG_CAP("(msg_type):ACCEPT, (slot_id):%llu", acc_slot);
      // IMPORTANT!!! if there is no such acceptor then init
      if (acceptors_.count(acc_slot) == 0) { 

        LOG_DEBUG_CAP("(msg_type):ACCEPT, New Acceptor");
        acceptors_[acc_slot] = new Acceptor(*view_);
      }

      MsgAckAccept *msg_ack_acc = acceptors_[acc_slot]->handle_msg_accept(msg_acc);
      commo_->send_one_msg(msg_ack_acc, ACCEPTED, msg_acc->msg_header().node_id());
      break;
    } 

    case ACCEPTED: {
      // proposer should handle ack of accept message
      // IMPORTANT! if curr_proposer_ == NULL Drop TODO can send other info
      if (!curr_proposer_) {
        LOG_DEBUG_CAP("(msg_type):ACCEPTED, Value has been chosen and Proposer is NULL now! Return!");
        return;
      }

      MsgAckAccept *msg_ack_acc = (MsgAckAccept *)msg; 

      if (msg_ack_acc->msg_header().slot_id() != max_chosen_ + 1) {
        LOG_DEBUG_CAP("(msg_type):ACCEPTED, This (slot_id):%llu is not (current_id):%llu! Return!", msg_ack_acc->msg_header().slot_id(), max_chosen_ + 1);
        return;
      }

      // handle_msg_accepted
      switch (curr_proposer_->handle_msg_accepted(msg_ack_acc)) {
        case DROP: break;
        case NOT_ENOUGH: break;
        case CHOOSE: {

          // First add the chosen_value into chosen_values_ 
          PropValue *chosen_value = curr_proposer_->get_chosen_value();

          LOG_INFO_CAP("*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*");
          LOG_INFO_CAP("Successfully Choose (value):%s !", chosen_value->data().c_str());
          LOG_INFO_CAP("*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*");

          // IMPORTANT 
          chosen_values_[max_chosen_ + 1] = chosen_value;
          // self increase max_chosen_
          max_chosen_++;
          LOG_DEBUG_CAP("(current_slot):%llu", max_chosen_);
          if (chosen_value->id() == curr_value_->id()) {
            // client's commit succeeded, if no value to commit, set NULL
            if (tocommit_values_.empty()) {
              LOG_INFO_CAP("Proposer END MISSION Temp");
              curr_proposer_ = NULL;
              return;
            }
            // start committing a new value from queue
            curr_value_->set_data(tocommit_values_.front());
            // pop the value
            tocommit_values_.pop();
            value_id_t value_id = curr_value_->id() + (1 << 16) + view_->whoami();
            curr_value_->set_id(value_id);
            delete curr_proposer_;
            // start a new slot
            new_slot();
          } else {
            // recommit the same value
            LOG_INFO_CAP("Recommit the same (value):%s!!!", curr_value_->data().c_str());
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
