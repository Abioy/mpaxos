/*
 * view.hpp
 *
 * Created on: May 08, 2015
 * Author: Lijing 
 */

#pragma once
#include "internal_types.hpp"
#include <set>
namespace mpaxos {

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
