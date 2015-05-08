/**
 * captain.hpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */

#include "commo.hpp"
#include "captain.hpp"

namespace mpaxos {
Commo::Commo(View &view, std::vector<Captain *> &captains) 
  : view_(&view), captains_(captains) {
}
Commo::~Commo() {
}
void Commo::broadcast_msg(google::protobuf::Message *msg, MsgType msg_type) {
  for (int i = 0; i < captains_.size(); i++) {
   captains_[i]->handle_msg(msg, msg_type);
  }
}
void Commo::send_one_msg(google::protobuf::Message *msg, MsgType msg_type, node_id_t node_id) {
  captains_[node_id]->handle_msg(msg,msg_type);
}
} // namespace mpaxos
