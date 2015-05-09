/*
 * view.hpp
 *
 * Created on: May 08, 2015
 * Author: Lijing 
 */
#include "view.hpp"
namespace mpaxos {

View::View(node_id_t node_id) : node_id_(node_id) {
  nodes_.insert(node_id);
}

std::set<node_id_t> * View::get_nodes() {
  return &nodes_;
}

node_id_t View::whoami() {
  return node_id_;
}
} //namespace mpaxos
