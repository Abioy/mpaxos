
#pragma once
#include "internal_types.hpp"
#include "mpaxos.pb.h"
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
} // namespace mpaxos


//#ifndef VIEW_H_
//#define VIEW_H_
//
//#include <stdbool.h>
//#include "mpaxos/mpaxos-types.h"
//#include <string>
//#include <map>
//
//typedef struct {
//    std::string name;  
//    std::string addr;
//    int32_t port;
//} host_info_t;
//
//class View {
//public:
//    std::set<node_id_t> nodes_;
//
//    node_id_t whoami();
//}
//
//typedef std::map<std::string, host_info_t> host_map_t;
//typedef host_map_t::iterator host_map_it_t;
////static host_info_t *my_host_info_t;
//
//void mpaxos_add_node(std::string hostname, std::string addr, int32_t port);
//
//host_info_t* mpaxos_node_info(std::string &hostname);
//
//host_map_t* mpaxos_get_all_nodes();
//
//host_info_t* mpaxos_whoami();
//
//void mpaxos_set_me(std::string &hostname);
//
//typedef struct {
//    nodeid_t nid;
//    int port;
//    char name[100];
//    char ip[100];
//} node_info_t;
//
//void view_init();
//
//void view_destroy();
//
//void set_nodename(const char *nodename);
//
//void set_node(const char* nodename, const char* addr, int port);
//
//int get_group_size(groupid_t gid);
//
//void set_local_nid(nodeid_t nid);
//
//nodeid_t get_local_nid();
//
//bool is_in_group(groupid_t gid);
//
//int set_gid_nid(groupid_t gid, nodeid_t nid);
//
//apr_array_header_t *get_group_nodes(groupid_t gid); 
//
//apr_hash_t* view_group_table(groupid_t);
//
//void get_all_groupids(groupid_t **gids, size_t *sz_gids);
//
//apr_array_header_t *get_view_dft(groupid_t gid);
//
//apr_array_header_t *get_view(groupid_t gid);
//#endif
//
