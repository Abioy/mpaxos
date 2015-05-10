/**
 * captain.hpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */
#pragma once
#include "view.hpp"
namespace mpaxos {
class Captain;
class Commo {
 public:
  Commo(std::vector<Captain *> &);
  ~Commo();
  void broadcast_msg(google::protobuf::Message *, MsgType);
  void send_one_msg(google::protobuf::Message *, MsgType, node_id_t);

 private:
//  View *view_;
  std::vector<Captain *> captains_;
};
} // namespace mpaxos
