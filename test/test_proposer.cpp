/**
 * test_proposer.cpp
 * To test Proposer Class
 * Author: Lijing Wang
 */

#include "proposer.hpp"
#include "acceptor.hpp"
#include <iostream>

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
  LOG_INFO("Test_Proposer Started Phase I");
  MsgPrepare *msg_pre = prop.msg_prepare();
  LOG_INFO("Test_Proposer (ballot_id_):%llu", msg_pre->ballot_id());
//  std::cout << "ballot_id_ " << msg_pre->ballot_id() <<std::endl;

  // Phase I Propsergot msg_pre and send to all acceptors
  for (it = nodes_set->begin(); it != nodes_set->end(); it++) {
      // send  
  }
  
  // listen to all acceptors got ack in different threads
  // fake acceptors here
  View view2(0);
  Acceptor acc(view2);
  MsgAckPrepare *msg_ack_pre = acc.handle_msg_prepare(msg_pre); 
  MsgAccept *msg_acc = NULL;
  switch (prop.handle_msg_promise(msg_ack_pre)) {
    case DROP: break;
    case NOT_ENOUGH: break;
    case CONTINUE: {
      // Send to all acceptors in view
      LOG_INFO("Test_Proposer Continue to Phase II");
      msg_acc = prop.msg_accept();
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
  //Not Good to avoid error
  if (!msg_acc) return -1;
  MsgAckAccept *msg_ack_acc = acc.handle_msg_accept(msg_acc);
  // handle_msg_accepted
  switch (prop.handle_msg_accepted(msg_ack_acc)) {
    case DROP: break;
    case NOT_ENOUGH: break;
    case CHOOSE: {
//      std::cout << "One Value is Successfully Chosen!" << std::endl;
      LOG_INFO("Test_Proposer One Value is Successfully Chosen!");
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

//INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv) {
//  LOG(INFO) << "My first info log using default logger -- info";
//  LOG(DEBUG) << "My first info log using default logger -- debug";
  return mpaxos::main(argc, argv);
}
