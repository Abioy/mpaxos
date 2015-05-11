/**
 * captain.hpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */

#include "commo.hpp"
#include "captain.hpp"
#include <iostream>

namespace mpaxos {
Commo::Commo(std::vector<Captain *> &captains) 
  : captains_(captains) {
}
Commo::~Commo() {
}

void Commo::broadcast_msg(google::protobuf::Message *msg, MsgType msg_type) {
  for (uint32_t i = 0; i < captains_.size(); i++) {
    std::cout << " --- Commo Broadcast to captain " << i << " MsgType: " << msg_type <<  std::endl;
    captains_[i]->handle_msg(msg, msg_type);
  }
}

void Commo::send_one_msg(google::protobuf::Message *msg, MsgType msg_type, node_id_t node_id) {
  std::cout << " --- Commo Send ONE to captain " << node_id << " MsgType: " << msg_type << std::endl;
  captains_[node_id]->handle_msg(msg,msg_type);
}
} // namespace mpaxos
