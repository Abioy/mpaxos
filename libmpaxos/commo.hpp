/**
 * captain.hpp
 * Author: Lijing Wang
 * Date: 5/5/2015
 */
#pragma once
#include "view.hpp"
#include "threadpool.hpp" 
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include "ThreadPool.h"
using namespace boost::threadpool;
namespace mpaxos {
class Captain;
class Commo {
 public:
  Commo(std::vector<Captain *> &);
  ~Commo();
  void broadcast_msg(google::protobuf::Message *, MsgType);
  void send_one_msg(google::protobuf::Message *, MsgType, node_id_t);
//  void set_pool(ThreadPool *);
  void set_pool(pool *);

 private:
//  View *view_;
  std::vector<Captain *> captains_;
  pool *pool_;
//  ThreadPool *pool_;
};
} // namespace mpaxos
