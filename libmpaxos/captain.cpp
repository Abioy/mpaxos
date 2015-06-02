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
  : view_(&view), max_chosen_(0), curr_proposer_(NULL), commo_(NULL), 
    callback_slot_(1), work_(true) {

  curr_value_ = new PropValue();
  curr_value_->set_id(view_->whoami());

  chosen_values_.push_back(NULL);

  acceptors_.push_back(NULL);
}

Captain::~Captain() {
}

void Captain::set_callback(callback_t& cb) { 
  callback_ = cb;
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
 * set set thread_pool handler 
 */
//void Captain::set_thread_pool(ThreadPool *pool) {
//  pool_ = pool; 
//}

/** 
 * client commits one value to captain
 */
void Captain::commit_value(std::string data) {
//  std::lock_guard<boost::mutex> lock(mutex_);
//  std::cout << "\nCaptain commit_value" << std::endl;
  LOG_DEBUG_CAP("<commit_value> Start");

  tocommit_values_mutex_.lock();
  LOG_DEBUG_CAP("(tocommit_values_.size):%lu", tocommit_values_.size());
  if (!tocommit_values_.empty()) {
    tocommit_values_.push(data);
    tocommit_values_mutex_.unlock();
    return;
  } 

  if (curr_proposer_) {
    tocommit_values_.push(data);
    tocommit_values_mutex_.unlock();
    return;
  }

  tocommit_values_mutex_.unlock();

  curr_value_mutex_.lock();
  curr_value_->set_data(data);
  LOG_DEBUG_CAP("(view_->whoami()):%u", view_->whoami());
  value_id_t value_id = curr_value_->id() + (1 << 16);
  curr_value_->set_id(value_id);
  LOG_DEBUG_CAP("(curr_value) id:%llu data:%s", curr_value_->id(), curr_value_->data().c_str());
  LOG_DEBUG_CAP("(curr_slot):%llu", max_chosen_ + 1);
  curr_value_mutex_.unlock();

  // start a new instance
  new_slot();

//  boost::unique_lock<boost::mutex> lck(commit_mutex_);
//  done_ = false;
//  while (!done_) commit_con_.wait(lck);

//  LOG_DEBUG_CAP("<commit_value> Over!");
  // clean curr_proposer_!!
//  clean();
}

/**
 * captain starts phaseI
 */
void Captain::new_slot() {
  // new proposer
  work_mutex_.lock();
  if (work_ == false) {
    LOG_DEBUG_CAP("%snew_slot I'm DEAD --NodeID %u", BAK_RED, view_->whoami());
    work_mutex_.unlock();
    return;
  }
  work_mutex_.unlock();

  LOG_TRACE_CAP("<new_slot> Start");

  curr_proposer_mutex_.lock();
  curr_proposer_ = new Proposer(*view_, *curr_value_);
  // new acceptor

  MsgPrepare *msg_pre = curr_proposer_->msg_prepare();
  // important!! captain set slot_id
  curr_proposer_mutex_.unlock();

//  max_chosen_mutex_.lock();
  msg_pre->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
//  max_chosen_mutex_.unlock();

  LOG_TRACE_CAP("<new_slot> call <broadcast_msg> with (msg_type):PREPARE");

  commo_->broadcast_msg(msg_pre, PREPARE);

  LOG_TRACE_CAP("<new_slot> call <broadcast_msg> Over");
}

/**
 * handle message from commo, all kinds of message
 */
//void Captain::handle_msg(google::protobuf::Message *msg, MsgType msg_type) {
////  pool_->enqueue(&Captain::handle_msg_thread, msg, msg_type);
//  pool_->enqueue(std::bind(&Captain::handle_msg_thread, this, msg, msg_type));
//}

/**
 * handle message from commo, all kinds of message
 */
void Captain::handle_msg(google::protobuf::Message *msg, MsgType msg_type) {

  work_mutex_.lock();
  if (work_ == false) {
    LOG_DEBUG_CAP("%sI'm DEAD --NodeID %u", BAK_RED, view_->whoami());
    work_mutex_.unlock();
    return;
  }
  work_mutex_.unlock();

  LOG_TRACE_CAP("<handle_msg> Start (msg_type):%d", msg_type);

  switch (msg_type) {

    case PREPARE: {
      // acceptor should handle prepare message
      MsgPrepare *msg_pre = (MsgPrepare *)msg;

      slot_id_t acc_slot = msg_pre->msg_header().slot_id();
      LOG_TRACE_CAP("(msg_type):PREPARE, (slot_id): %llu", acc_slot);
      // IMPORTANT!!! if there is no such acceptor then init

      acceptors_mutex_.lock();
      for (int i = acceptors_.size(); i <= acc_slot; i++) {
        LOG_TRACE_CAP("(msg_type):PREPARE, New Acceptor");
        acceptors_.emplace_back(new Acceptor(*view_));
      }

      MsgAckPrepare * msg_ack_pre = acceptors_[acc_slot]->handle_msg_prepare(msg_pre);
      acceptors_mutex_.unlock();

      commo_->send_one_msg(msg_ack_pre, PROMISE, msg_pre->msg_header().node_id());
      break;
    }

    case PROMISE: {
      // proposer should handle ack of prepare message
      // IMPORTANT! if curr_proposer_ == NULL Drop TODO can send other info


      MsgAckPrepare *msg_ack_pre = (MsgAckPrepare *)msg;

  //    max_chosen_mutex_.lock();
      if (msg_ack_pre->msg_header().slot_id() != max_chosen_ + 1) {
        LOG_TRACE_CAP("(msg_type):PROMISE, This (slot_id):%llu is not (current_id):%llu! Return!", 
                      msg_ack_pre->msg_header().slot_id(), max_chosen_ + 1);
  //      max_chosen_mutex_.unlock();
        return;
      }
  //    max_chosen_mutex_.unlock();

      curr_proposer_mutex_.lock();
      if (!curr_proposer_) {
        LOG_TRACE_CAP("(msg_type):PROMISE, Value has been chosen and Proposer is NULL now! Return!");
        curr_proposer_mutex_.unlock();
        return;
      }

      switch (curr_proposer_->handle_msg_promise(msg_ack_pre)) {
        case DROP: {
          curr_proposer_mutex_.unlock();
          break;
        }
        case NOT_ENOUGH: {
          curr_proposer_mutex_.unlock();
          break;
        }        
        case CONTINUE: {
          // Send to all acceptors in view
          LOG_TRACE_CAP("(msg_type):PROMISE, Continue to Phase II");
          MsgAccept *msg_acc = curr_proposer_->msg_accept();

//          max_chosen_mutex_.lock();
          msg_acc->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
//          max_chosen_mutex_.unlock();

          curr_proposer_mutex_.unlock();

          commo_->broadcast_msg(msg_acc, ACCEPT);
          break;
        }
        case RESTART: {  //RESTART
          MsgPrepare *msg_pre = curr_proposer_->restart_msg_prepare();

//          max_chosen_mutex_.lock();
          msg_pre->mutable_msg_header()->set_slot_id(max_chosen_ + 1);
//          max_chosen_mutex_.unlock();

          curr_proposer_mutex_.unlock();

          commo_->broadcast_msg(msg_pre, PREPARE);
        }
        default: {
          curr_proposer_mutex_.unlock();
        }
      }
      break;
    }
                
    case ACCEPT: {
      // acceptor should handle accept message
      MsgAccept *msg_acc = (MsgAccept *)msg;
      slot_id_t acc_slot = msg_acc->msg_header().slot_id();

      LOG_TRACE_CAP("(msg_type):ACCEPT, (slot_id):%llu", acc_slot);
      // IMPORTANT!!! if there is no such acceptor then init

      acceptors_mutex_.lock();
      for (int i = acceptors_.size(); i <= acc_slot; i++) {
        LOG_TRACE_CAP("(msg_type):PREPARE, New Acceptor");
        acceptors_.emplace_back(new Acceptor(*view_));
      }

      MsgAckAccept *msg_ack_acc = acceptors_[acc_slot]->handle_msg_accept(msg_acc);
      acceptors_mutex_.unlock();

      commo_->send_one_msg(msg_ack_acc, ACCEPTED, msg_acc->msg_header().node_id());
      break;
    } 

    case ACCEPTED: {
      // proposer should handle ack of accept message
      // IMPORTANT! if curr_proposer_ == NULL Drop TODO can send other info

      MsgAckAccept *msg_ack_acc = (MsgAckAccept *)msg; 

//      max_chosen_mutex_.lock();
      if (msg_ack_acc->msg_header().slot_id() != max_chosen_ + 1) {
        LOG_TRACE_CAP("(msg_type):ACCEPTED, This (slot_id):%llu is not (current_id):%llu! Return!", 
                      msg_ack_acc->msg_header().slot_id(), max_chosen_ + 1);
//        max_chosen_mutex_.unlock();
        return;
      }
//      max_chosen_mutex_.unlock();

      // handle_msg_accepted
      curr_proposer_mutex_.lock();
      if (!curr_proposer_) {
        LOG_TRACE_CAP("(msg_type):ACCEPTED, Value has been chosen and Proposer is NULL now! Return!");
        curr_proposer_mutex_.unlock();
        return;
      }
      AckType type = curr_proposer_->handle_msg_accepted(msg_ack_acc);

      switch (type) {
        case DROP: {
          curr_proposer_mutex_.unlock();
          break;
        }

        case NOT_ENOUGH: {
          curr_proposer_mutex_.unlock();
          break;
        }        
        
        case CHOOSE: {

          // First add the chosen_value into chosen_values_ 
          PropValue *chosen_value = curr_proposer_->get_chosen_value();
          curr_proposer_ = NULL;
          LOG_DEBUG_CAP("%sNodeID:%u Successfully Choose (value):%s ! (slot_id):%llu %s", 
                        BAK_MAG, view_->whoami(), chosen_value->data().c_str(), max_chosen_ + 1, NRM);
          curr_proposer_mutex_.unlock();

          add_chosen_value(max_chosen_ + 1, chosen_value);
          // self increase max_chosen_ when to increase

          LOG_DEBUG_CAP("(max_chosen_):%llu (chosen_values.size()):%lu", max_chosen_, chosen_values_.size());
          LOG_DEBUG_CAP("(msg_type):ACCEPTED, Broadcast this chosen_value");
          MsgDecide *msg_dec = msg_decide(max_chosen_);

          commo_->broadcast_msg(msg_dec, DECIDE);

          if (chosen_value->id() == curr_value_->id()) {
            // Teach Progress to help others fast learning

            // client's commit succeeded, if no value to commit, set NULL

            // start committing a new value from queue
            tocommit_values_mutex_.lock();
            if (tocommit_values_.empty()) {
              LOG_DEBUG_CAP("Proposer END MISSION Temp");
              tocommit_values_mutex_.unlock();
              return;
            }
            std::string data = tocommit_values_.front();
            // pop the value
            tocommit_values_.pop();
            tocommit_values_mutex_.unlock();


            curr_value_mutex_.lock();
            curr_value_->set_data(data);
            value_id_t value_id = curr_value_->id() + (1 << 16);
            curr_value_->set_id(value_id);
            curr_value_mutex_.unlock();

            new_slot();
          } else {
            // recommit the same value
            LOG_TRACE_CAP("Recommit the same (value):%s!!!", curr_value_->data().c_str());
            new_slot();
          }
          break;
        }

        default: { //RESTART
          LOG_DEBUG_CAP("--NodeID:%u (msg_type):ACCEPTED, %sRESTART!%s", view_->whoami(), TXT_RED, NRM); 
//          MsgPrepare *msg_pre = curr_proposer_->restart_msg_prepare();
          curr_proposer_mutex_.unlock();
          new_slot();          
        }
      }
      break;
    }

    case DECIDE: {
      // captain should handle this message
      MsgDecide *msg_dec = (MsgDecide *)msg;
      slot_id_t dec_slot = msg_dec->msg_header().slot_id();

//      max_chosen_mutex_.lock();
      if (max_chosen_ >= dec_slot && chosen_values_[dec_slot]) {
//      max_chosen_mutex_.unlock();
        return;
      }
//      max_chosen_mutex_.unlock();

      LOG_DEBUG_CAP("%s(msg_type):DECIDE (slot_id):%llu from (node_id):%u --NodeID %u handle", 
                    UND_RED, dec_slot, msg_dec->msg_header().node_id(), view_->whoami());

      acceptors_mutex_.lock();
      if (acceptors_.size() > dec_slot && acceptors_[dec_slot]->get_max_value() && 
         (acceptors_[dec_slot]->get_max_value()->id() == msg_dec->value_id())) {
        // the value is stored in acceptors_[dec_slot]->max_value_
        acceptors_mutex_.unlock();
        add_chosen_value(dec_slot, acceptors_[dec_slot]->get_max_value()); 

        if (if_recommit()) {
        LOG_DEBUG_CAP("%sRECOMMIT! (msg_type):DECIDE (slot_id):%llu from (node_id):%u --NodeID %u handle", 
              UND_RED, dec_slot, msg_dec->msg_header().node_id(), view_->whoami());
        new_slot();
        }
      } else {
        // acceptors_[dec_slot] doesn't contain such value, need learn from this sender
        acceptors_mutex_.unlock();
        MsgLearn *msg_lea = msg_learn(dec_slot);
        commo_->send_one_msg(msg_lea, LEARN, msg_dec->msg_header().node_id());
      } 
      break;
    }

    case LEARN: {
      // captain should handle this message
      MsgLearn *msg_lea = (MsgLearn *)msg;
      slot_id_t lea_slot = msg_lea->msg_header().slot_id();

      LOG_DEBUG_CAP("%s(msg_type):LEARN (slot_id):%llu from (node_id):%u --NodeID %u handle", 
                    UND_GRN, lea_slot, msg_lea->msg_header().node_id(), view_->whoami());

//      max_chosen_mutex_.lock();
      if (lea_slot > max_chosen_ || chosen_values_[lea_slot] == NULL) {
//        max_chosen_mutex_.unlock();
        return;
      }
//      max_chosen_mutex_.unlock();

      MsgTeach *msg_tea = msg_teach(lea_slot);
      commo_->send_one_msg(msg_tea, TEACH, msg_lea->msg_header().node_id());
      break;
    }

    case TEACH: {
      // captain should handle this message
      MsgTeach *msg_tea = (MsgTeach *)msg;
      slot_id_t tea_slot = msg_tea->msg_header().slot_id();

//      max_chosen_mutex_.lock();
      if (max_chosen_ >= tea_slot && chosen_values_[tea_slot]) {
//        max_chosen_mutex_.unlock();
        return;
      }
//      max_chosen_mutex_.unlock();

      LOG_DEBUG_CAP("%s(msg_type):TEACH (slot_id):%llu from (node_id):%u --NodeID %u handle", 
                    UND_YEL, tea_slot, msg_tea->msg_header().node_id(), view_->whoami());
      // only when has value
//      if (msg_tea->mutable_prop_value())
      add_chosen_value(tea_slot, msg_tea->mutable_prop_value());

      if (if_recommit()) {
      new_slot();
      }       
      break;
    }

    default: 
      break;
  }
}



/**
 * Return Msg_header
 */
MsgHeader *Captain::set_msg_header(MsgType msg_type) {
  MsgHeader *msg_header = new MsgHeader();
  msg_header->set_msg_type(msg_type);
  msg_header->set_node_id(view_->whoami());

//  max_chosen_mutex_.lock();
  msg_header->set_slot_id(max_chosen_ + 1);
//  max_chosen_mutex_.unlock();

  return msg_header;
}

/**
 * Return Msg_header II
 */
MsgHeader *Captain::set_msg_header(MsgType msg_type, slot_id_t slot_id) {
  MsgHeader *msg_header = new MsgHeader();
  msg_header->set_msg_type(msg_type);
  msg_header->set_node_id(view_->whoami());
  msg_header->set_slot_id(slot_id);
  return msg_header;
}

/**
 * Return Decide Message
 */
MsgDecide *Captain::msg_decide(slot_id_t slot_id) {
  MsgHeader *msg_header = set_msg_header(MsgType::DECIDE, slot_id);
  MsgDecide *msg_dec = new MsgDecide();
  msg_dec->set_allocated_msg_header(msg_header); 
//  msg_dec->set_value_id(curr_proposer_->get_chosen_value()->id());
//  chosen_values_mutex_.lock();
  msg_dec->set_value_id(chosen_values_[slot_id]->id());
//  chosen_values_mutex_.unlock();
  return msg_dec;
}

/**
 * Return Learn Message
 */
MsgLearn *Captain::msg_learn(slot_id_t slot_id) {
  MsgHeader *msg_header = set_msg_header(MsgType::LEARN, slot_id);
  MsgLearn *msg_lea = new MsgLearn();
  msg_lea->set_allocated_msg_header(msg_header);
  return msg_lea;
}

/**
 * Return Teach Message
 */
MsgTeach *Captain::msg_teach(slot_id_t slot_id) {
  MsgHeader *msg_header = set_msg_header(MsgType::TEACH, slot_id);
  MsgTeach *msg_tea = new MsgTeach();
  msg_tea->set_allocated_msg_header(msg_header);
//  PropValue *prop_value = NULL;  
//  if (slot_id == max_chosen_ + 1)
//    prop_value = curr_proposer_->get_chosen_value();
//  else if (slot_id < max_chosen_ + 1)
//  prop_value = chosen_values_[slot_id];
//  chosen_values_mutex_.lock();
  msg_tea->set_allocated_prop_value(chosen_values_[slot_id]);
//  chosen_values_mutex_.unlock();
  return msg_tea; 
}

/** 
 * Callback function after commit_value  
 */
void Captain::clean() {
  curr_proposer_mutex_.lock();
  if (curr_proposer_) { 
//    delete curr_proposer_;
    curr_proposer_ = NULL;
  }
  curr_proposer_mutex_.unlock();
}

void Captain::crash() {
  work_mutex_.lock();
  work_ = false;
//  curr_proposer_mutex_.lock();
//  curr_proposer_ = NULL;
//  curr_proposer_mutex_.unlock();
  work_mutex_.unlock();
}

void Captain::recover() {
  work_mutex_.lock();
  work_ = true;
  work_mutex_.unlock();
}

bool Captain::get_status() {
  return work_;
}

void Captain::print_chosen_values() {
  LOG_INFO_CAP("%s%sNodeID:%u (chosen_values_): %s", BAK_BLU, TXT_WHT, view_->whoami(), NRM);
  if (chosen_values_.size() == 1) {
     LOG_INFO_CAP("%sEMPTY!%s", BLD_RED, NRM); 
  }
  for (uint64_t i = 1; i < chosen_values_.size(); i++) {
    if (chosen_values_[i] != NULL) {
      LOG_INFO_CAP("%s%s(slot_id):%llu (value) id:%llu data: %s%s", 
                     BAK_CYN, TXT_WHT, i, chosen_values_[i]->id(), chosen_values_[i]->data().c_str(), NRM);
    } else {
      LOG_INFO_CAP("%s%s(slot_id):%llu (value):NULL%s", BAK_CYN, TXT_WHT, i, NRM); 
    }
  }
}

std::vector<PropValue *> Captain::get_chosen_values() {
  return chosen_values_; 
}

bool Captain::if_recommit() {
//  curr_proposer_mutex_.lock();
  if (curr_proposer_ == NULL) {
//    curr_proposer_mutex_.unlock();
    return false;
  }
  for (int i = 1; i < chosen_values_.size(); i++) {
    if (chosen_values_[i] && chosen_values_[i]->id() == curr_value_->id()) {
//      curr_proposer_mutex_.unlock();
      return false;
    }
  }
//  curr_proposer_mutex_.unlock();
  return true;
}

/**
 * Add a new chosen_value 
 */
void Captain::add_chosen_value(slot_id_t slot_id, PropValue *prop_value) {
  max_chosen_mutex_.lock();

  for (int i = max_chosen_; i < slot_id; i++) {
    chosen_values_.push_back(NULL);
  }

  // Only when this slot doesn't contain a value, add one
  if (chosen_values_[slot_id] == NULL) {
    chosen_values_[slot_id] = new PropValue(*prop_value);
    if (slot_id > max_chosen_) {
      max_chosen_ = slot_id; 
      LOG_DEBUG("<add_chosen_value> (max_chosen_): %llu", max_chosen_);
    }
    callback_mutex_.lock();
    while (callback_slot_ <= max_chosen_) {
      if (chosen_values_[callback_slot_] == NULL) {
        break;
      }
      callback_(slot_id, *(prop_value->mutable_data()));
      callback_slot_++;
    } 
    callback_mutex_.unlock();
  }

  max_chosen_mutex_.unlock();
}

} //  namespace mpaxos
