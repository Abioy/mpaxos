/**
 * commo.cpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */

#include "commo.hpp"
#include "captain.hpp"
#include <iostream>

namespace mpaxos {
// Old Version
Commo::Commo(std::vector<Captain *> &captains) 
  : captains_(captains), context_(1), receiver_(context_, ZMQ_DEALER) {
//  for (uint32_t i = 0; i < captains_.size(); i++) {
////    senders_.push_back(new zmq::socket_t(context_, ZMQ_DEALER));
//  }
//  pool_ = new pool(4);
}

Commo::Commo(Captain *captain, View &view, pool *pl) 
  : captain_(captain), view_(&view), pool_(pl), context_(1), 
    receiver_(context_, ZMQ_DEALER) {
  for (uint32_t i = 0; i < view_->get_nodes()->size(); i++) {
    senders_.push_back(new zmq::socket_t(context_, ZMQ_DEALER));
    std::string address = "tcp://localhost:555" + std::to_string(i);
//    LOG_INFO_COM("Connect to address %s", address.c_str());
    senders_[i]->connect(address.c_str());
  }
  self_pool_ = new pool(1);
  std::string address = "tcp://*:555" + std::to_string(view_->whoami());
  LOG_INFO_COM("My address %s", address.c_str());
  receiver_.bind (address.c_str());
//  std::thread listen(boost::bind(&Commo::waiting_msg, this)); 
  self_pool_->schedule(boost::bind(&Commo::waiting_msg, this));
}

Commo::~Commo() {
}

void Commo::waiting_msg() {
  while (true) {
    LOG_TRACE_COM("I'm waiting -- Node_ID_%u", view_->whoami());
    zmq::message_t request;
    //  Wait for next request from client
    receiver_.recv (&request);
    LOG_TRACE_COM("Received-- Node_ID_%u", view_->whoami());
   
    std::string msg_str(static_cast<char*>(request.data()), request.size());
    int type = int(msg_str.back() - '0');
    google::protobuf::Message *msg = nullptr;
//    LOG_INFO_COM("type %d", type);
    switch(type) {
      case PREPARE: {
        msg = new MsgPrepare();
        break;
      }
      case PROMISE: {
        msg = new MsgAckPrepare();
        break;
      }
      case ACCEPT: {
        msg = new MsgAccept();
        break;
      }
      case ACCEPTED: {
        msg = new MsgAckAccept();
        break;
      }
      case DECIDE: {
        msg = new MsgDecide();
        break;
      }
      case LEARN: {
        msg = new MsgLearn();
        break;
      }                        
      case TEACH: {
        msg = new MsgTeach();
        break;
      }
    }
    msg_str.pop_back();
    msg->ParseFromString(msg_str);
    pool_->schedule(boost::bind(&Captain::handle_msg, captain_, msg, static_cast<MsgType>(type)));
//    captain_->handle_msg(msg, static_cast<MsgType>(type));

//    std::string text_str;
//    google::protobuf::TextFormat::PrintToString(*msg, &text_str);
//    LOG_INFO_COM("%s", text_str.c_str());
  }
}

//void Commo::set_pool(ThreadPool *pool) {
void Commo::set_pool(pool *pl) {
  pool_ = pl;
}

void Commo::broadcast_msg(google::protobuf::Message *msg, MsgType msg_type) {

  for (uint32_t i = 0; i < view_->get_nodes()->size(); i++) {
    LOG_INFO_COM("Broadcast to --Captain %u (msg_type):%d", i, msg_type);
    std::string msg_str;
    msg->SerializeToString(&msg_str);
    msg_str.append(std::to_string(msg_type));
    // create a zmq message from the serialized string
    zmq::message_t request(msg_str.size());
    memcpy((void *)request.data(), msg_str.c_str(), msg_str.size());
    senders_[i]->send(request);
  }

}

void Commo::send_one_msg(google::protobuf::Message *msg, MsgType msg_type, node_id_t node_id) {
//  std::cout << " --- Commo Send ONE to captain " << node_id << " MsgType: " << msg_type << std::endl;
  LOG_INFO_COM("Send ONE to --Captain %u (msg_type):%d", node_id, msg_type);

  std::string msg_str;
  msg->SerializeToString(&msg_str);
  msg_str.append(std::to_string(msg_type));
  // create a zmq message from the serialized string
  zmq::message_t request(msg_str.size());
  memcpy((void *)request.data(), msg_str.c_str(), msg_str.size());

  senders_[node_id]->send(request);
}
} // namespace mpaxos
