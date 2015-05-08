/*
 * acceptor.hpp
 *
 * Created on: April 30, 2015
 * Author: Lijing 
 */

#include "acceptor.hpp"
namespace mpaxos {
Acceptor::Acceptor(View &view) 
  : max_proposed_ballot_(0), max_accepted_ballot_(0), max_value_(NULL) {
}
Acceptor::~Acceptor() {
}
/** 
 * handle prepare requests from proposers:
 * if ballot_id > max_proposed_ballot_, 
 *   update max_proposed_ballot_,
 *   reply yes with max_value_;
 * else 
 *   reply no.
 * return MsgAckPrepare *
 */ 
MsgAckPrepare *Acceptor::handle_msg_prepare(MsgPrepare *msg_pre) {
  ballot_id_t curr_ballot = msg_pre->ballot_id();
  std::cout << "Acceptor -- Phase I : Inside acceptor handle_msg_prepare curr_ballot: " 
            << curr_ballot << std::endl;
  MsgHeader *msg_header = new MsgHeader();
  msg_header->set_msg_type(MsgType::PROMISE);
  msg_header->set_node_id(view_->whoami());
  msg_header->set_slot_id(msg_pre->msg_header().slot_id());
  // prepare the msg_ack_prepare
  MsgAckPrepare *msg_ack_pre = new MsgAckPrepare();
  msg_ack_pre->set_allocated_msg_header(msg_header);
  msg_ack_pre->set_ballot_id(curr_ballot);
  msg_ack_pre->set_max_ballot_id(max_accepted_ballot_);
  if (curr_ballot > max_proposed_ballot_) {
    std::cout << "Return true" << std::endl;
    max_proposed_ballot_ = curr_ballot;
    msg_ack_pre->set_reply(true);
    if (max_value_) {
      msg_ack_pre->set_allocated_max_prop_value(max_value_);
    }  
  } else
    msg_ack_pre->set_reply(false);
  return msg_ack_pre;
}
/**
 * handle accept requests from proposers:
 * if ballot_id > max_proposed_ballot_, accept this value
 *   update max_proposed_ballot_, max_accepted_ballot_ and max_value_,
 *   reply yes;
 * else
 *   reply no.
 * return MsgAckAccept *
 */
MsgAckAccept *Acceptor::handle_msg_accept(MsgAccept *msg_acc) {
  ballot_id_t curr_ballot = msg_acc->ballot_id();
  // prepare msg_header
  MsgHeader *msg_header = new MsgHeader();
  msg_header->set_msg_type(MsgType::ACCEPTED);
  msg_header->set_node_id(view_->whoami());
  msg_header->set_slot_id(msg_acc->msg_header().slot_id());
  // prepare the msg_ack_accept
  MsgAckAccept *msg_ack_acc = new MsgAckAccept();
  msg_ack_acc->set_allocated_msg_header(msg_header);
  msg_ack_acc->set_ballot_id(curr_ballot);
  if (curr_ballot > max_proposed_ballot_) {
    max_proposed_ballot_ = curr_ballot;
    max_accepted_ballot_ = curr_ballot;
    max_value_ = msg_acc->mutable_prop_value();
    msg_ack_acc->set_reply(true);
  } else msg_ack_acc->set_reply(true);
  return msg_ack_acc;
}
}  // namespace mpaxos