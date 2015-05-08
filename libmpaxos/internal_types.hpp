#pragma once
#include <set>
#include <cstdint>
#include <queue>
#include "mpaxos.pb.h"
#include <iostream>
#include <thread>
#include <mutex>

namespace mpaxos {
enum AckType {
  DROP = 0,
  NOT_ENOUGH = 1,
  CONTINUE = 2,
  RESTART = 3,
  CHOOSE = 4
};

using node_id_t = uint16_t;
using slot_id_t = uint64_t;
using ballot_id_t = uint64_t;
using value_id_t = uint64_t;

class View {
 public:
//  View(set<node_id_t> &nodes);
  View(node_id_t node_id);
  std::set<node_id_t> * get_nodes();
  node_id_t whoami();
 private:
  std::set<node_id_t> nodes_;
  node_id_t node_id_;
};
}  // namespace mpaxos
