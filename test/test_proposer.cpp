/**
 * test_proposer.cpp
 * To test Proposer Class
 * Author: Lijing Wang
 */

#include "proposer.hpp"
namespace mpaxos {
int main(int argc, char** argv) {
  // init Proposer
  View view(0);
  std::set<node_id_t> *nodes_set = view.get_nodes();
  std::set<node_id_t>::iterator it;
  PropValue value;
  value.set_data("Hello World!");
  Proposer prop(view, value);  
  // Phase I Propser call prepare
  MsgPrepare *msg_pre = prop.msg_prepare();
  std::cout << "Started Phase I" << std::endl;

  // Phase I Propsergot msg_pre and send to all acceptors
  for (it = nodes_set->begin(); it != nodes_set->end(); it++) {
      // send  
  }
  
  // listen to all acceptors got ack in different threads
  // fake acceptors here
  // prepare msg_header
  MsgHeader *msg_header_pre = new MsgHeader();
  msg_header_pre->set_msg_type(MsgType::PROMISE);
  msg_header_pre->set_node_id(view.whoami());
  msg_header_pre->set_slot_id(0);
  // prepare the msg_ack_prepare
  MsgAckPrepare *msg_ack_pre = new MsgAckPrepare();
  msg_ack_pre->set_allocated_msg_header(msg_header_pre);
  msg_ack_pre->set_ballot_id(1);
  msg_ack_pre->set_reply(true);
  msg_ack_pre->set_max_ballot_id(0);

  switch (prop.handle_msg_promise(msg_ack_pre)) {
    case DROP: break;
    case NOT_ENOUGH: break;
    case CONTINUE: {
      // Send to all acceptors in view
      std::cout << "Continue to Phase II" << std::endl;
      MsgAccept *msg_acc = prop.msg_accept();
      for (it = nodes_set->begin(); it != nodes_set->end(); it++) {
        // send msg_accpect
      }
      break;
    }
    default: { //RESTART
      msg_pre = prop.restart_msg_prepare();
      for (it = nodes_set->begin(); it != nodes_set->end(); it++) {
        // send msg_prepare
      }
    }
  }
  // prepare msg_header
  MsgHeader *msg_header_acc = new MsgHeader();
  msg_header_acc->set_msg_type(MsgType::ACCEPTED);
  msg_header_acc->set_node_id(view.whoami());
  msg_header_acc->set_slot_id(0);
  // prepare the msg_ack_accept
  MsgAckAccept *msg_ack_acc = new MsgAckAccept();
  msg_ack_acc->set_allocated_msg_header(msg_header_acc);
  msg_ack_acc->set_ballot_id(1);
  msg_ack_acc->set_reply(true);
  // handle_msg_accepted
  switch (prop.handle_msg_accepted(msg_ack_acc)) {
    case DROP: break;
    case NOT_ENOUGH: break;
    case CHOOSE: {
      std::cout << "One Value is Successfully Chosen!" << std::endl;
      // destroy all the threads retlated to this paxos_instance 
      // TODO 
      break;
    }
    default: { //RESTART
      msg_pre = prop.restart_msg_prepare();
      for (it = nodes_set->begin(); it != nodes_set->end(); it++) {
        // send msg_prepare
      }
    }
  }
  return 0;
} 
}

int main(int argc, char** argv) {
  return mpaxos::main(argc, argv);
}
