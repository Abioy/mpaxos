
#include "commo.hpp"
#include "include_all.h"
#include <iostream>

static apr_pool_t *mp_comm_;
static apr_hash_t *ht_sender_; //nodeid_t -> sender_t
static apr_thread_mutex_t *mx_comm_;
static std::map<std::string, client_t *> client_map_;

server_t *server_ = NULL;

//View

//uint64_t recv_data_count = 0;
//uint64_t recv_msg_count = 0;
//time_t recv_start_time = 0;
//time_t recv_last_time = 0;
//time_t recv_curr_time = 0;

funid_t ADD = 1;

typedef struct {
    uint32_t a;
    uint32_t b;
} struct_add;

rpc_state* add(rpc_state *state) {
    struct_add *sa = (struct_add *)state->raw_input;
    uint32_t c = sa->a + sa->b;

	LOG_INFO("server add a:%d + b:%d\n",sa->a, sa->b);
    state->raw_output = (uint8_t*)malloc(sizeof(uint32_t));
    state->sz_output = sizeof(uint32_t);
    memcpy(state->raw_output, &c, sizeof(uint32_t));
    return NULL;
}

rpc_state* add_cb(rpc_state *state) {
    // Do nothing
    
    uint32_t *res = (uint32_t*) state->raw_input;
	uint32_t k = *res;
    
    LOG_INFO("client callback exceuted result:%d. \n",k);
    return NULL;
}

void mpaxos_commo_start() {

	//apr_initialize();

	poll_mgr_t *mgr= NULL;

	host_info_t *myself = mpaxos_whoami();

    signal(SIGPIPE, SIG_IGN);

    rpc_init();

    poll_mgr_create(&mgr, 1);  
    server_create(&server_, mgr);    

    strcpy(server_->comm->ip, myself->addr.c_str());
    server_->comm->port = myself->port;
    server_reg(server_, ADD, (void*)add); 
    server_start(server_);
	std::cout << "server info-- addr: " << myself ->addr << " port:  "
	   	<< myself->port << std::endl;
    printf("server started.\n");


	host_map_t *allnodes = mpaxos_get_all_nodes();
	for(host_map_it_t it = allnodes->begin(); it != allnodes->end(); it++) {
		
		client_map_[it->first] = NULL; 
		client_create(&client_map_[it->first], mgr);
		strcpy(client_map_[it->first]->comm->ip, it->second.addr.c_str());
		client_map_[it->first]->comm->port = it->second.port;
		client_reg(client_map_[it->first], ADD, (void*)add_cb);
		std::cout << it->first << " client created" <<std::endl;
		client_connect(client_map_[it->first]);

		std::cout << it->first << " client connected" <<std::endl;
	}
	
	std::cout << myself->name << ": all client connected" <<std::endl;
//	rpc_destroy();
}

// No Test
void mpaxos_commo_stop() {

	server_destroy(server_);
	
	for(std::map<std::string, client_t *>::iterator it = client_map_.begin();
		   	it != client_map_.end(); it++) {
		client_destroy(it->second);				
	}
    rpc_destroy();
}

void mpaxos_commo_sendto(std::string& hostname, msg_type_t msg_type,
        const uint8_t* data, size_t sz) {
	client_call(client_map_[hostname], msg_type, data, sz);
	std::cout << " client called -- hostname:" << hostname <<std::endl;
}

void mpaxos_commo_sendto_all(msg_type_t msg_type, const uint8_t *data, size_t sz) {

	for(std::map<std::string, client_t *>::iterator it = client_map_.begin();
		   	it != client_map_.end(); it++) {
		client_call(it->second, msg_type, data, sz);
	}
}

void comm_init() {
    apr_pool_create(&mp_comm_, NULL);
    ht_sender_ = apr_hash_make(mp_comm_);
    apr_thread_mutex_create(&mx_comm_, APR_THREAD_MUTEX_UNNESTED, mp_comm_);
    rpc_init();
}

void comm_destroy() {
    rpc_destroy();
    LOG_DEBUG("stopped listening on network.");

    // destroy all senders and recvrs
    apr_array_header_t *arr_nid = get_view(1);
    SAFE_ASSERT(arr_nid != NULL);
    for (int i = 0; i < arr_nid->nelts; i++) {
        nodeid_t nid = ((nodeid_t*)arr_nid->elts)[i];    
        client_t *c = NULL;
        c = (client_t*)apr_hash_get(ht_sender_, &nid, sizeof(nodeid_t));
        client_destroy(c);
    }

    if (server_) {
        server_destroy(server_);
    }

    apr_thread_mutex_destroy(mx_comm_);
    apr_pool_destroy(mp_comm_);
}

void send_to(nodeid_t nid, msg_type_t type, const uint8_t *data,
    size_t sz) {
    client_t *cli;
    apr_thread_mutex_lock(mx_comm_);
    SAFE_ASSERT(nid != 0);
    cli = (client_t*)apr_hash_get(ht_sender_, &nid, sizeof(nid));
    apr_thread_mutex_unlock(mx_comm_);
    //int hash_size = apr_hash_count(sender_ht_);

    SAFE_ASSERT(cli != NULL);
    client_call(cli, type, data, sz);
}

slotid_t send_to_slot_mgr(groupid_t gid, nodeid_t nid, uint8_t *data,
        size_t sz) {
/*
    sender_t *s_ptr;
    s_ptr = apr_hash_get(ht_sender_, &nid, sizeof(nid));

    sender_t *s = (sender_t *)malloc(sizeof(sender_t));
    strcpy(s->addr, s_ptr->addr);
    s->port = s_ptr->port;
    sender_init(s);

    uint8_t* buf = (uint8_t*) malloc(100);
    mpaxos_send_recv(s, data, sz, buf, 100); 
    slotid_t sid = (slotid_t)*buf;
    free(buf);
    free(s);
    return sid; 
*/
    return 0;
}
/**
 * thread safe.
 * @param gid
 * @param type
 * @param buf
 * @param sz
 */
void send_to_group(groupid_t gid, 
		   msg_type_t type, 
		   const uint8_t *buf,
		   size_t sz) {
	// TODO [FIX] this is not thread safe because of apache hash table
//  apr_hash_t *nid_ht = apr_hash_get(gid_nid_ht_ht_, &gid, sizeof(gid));
    apr_array_header_t *arr_nid = get_view(gid);
    SAFE_ASSERT(arr_nid != NULL);

    for (int i = 0; i < arr_nid->nelts; i++) {
        nodeid_t nid = ((nodeid_t*)arr_nid->elts)[i];
        send_to(nid, type, buf, sz);
    }
}

void connect_all_senders() {
    // [FIXME]
    apr_array_header_t *arr_nid = get_view(1);
    SAFE_ASSERT(arr_nid != NULL);
    
    LOG_TRACE("length of arr_nid: %d", (int32_t) arr_nid->nelts);
    for (int i = 0; i < arr_nid->nelts; i++) {
        nodeid_t nid = ((nodeid_t *)arr_nid->elts)[i];
	LOG_TRACE("get client for node id: %x", (int32_t) nid);
	SAFE_ASSERT(nid != 0);
        client_t *c = NULL;
        c = (client_t*) apr_hash_get(ht_sender_, &nid, sizeof(nodeid_t));
	SAFE_ASSERT(c != NULL);
        client_connect(c);
    }
}

void send_to_groups(groupid_t* gids, size_t sz_gids,
        msg_type_t type, const char *buf, size_t sz) {
    groupid_t gid = gids[0];
    send_to_group(gid, type, (const uint8_t *)buf, sz);
    // TODO [IMPROVE] Optimize
//    for (uint32_t i = 0; i < sz_gids; i++, gids++) {
//        gid = *gids;
//        send_to_group(gid, (const uint8_t *)buf, sz);
//    }
}

//rpc_state* on_prepare(rpc_state* state) {
//    msg_prepare_t *msg_prep;
//    msg_prep = mpaxos__msg_prepare__unpack(NULL, state->sz_input, state->raw_input);
//    //    log_message_rid("receive", "PREPARE", msg_prep->h, msg_prep->rids, 
//    //        msg_prep->n_rids, state->sz);
//    rpc_state* ret_state = handle_msg_prepare(msg_prep);
//    mpaxos__msg_prepare__free_unpacked(msg_prep, NULL);
//
//    state->raw_output = (uint8_t*) malloc(ret_state->sz_output);
//    state->sz_output = ret_state->sz_output;
//    memcpy(state->raw_output, ret_state->raw_output, ret_state->sz_output);
//    free(ret_state->raw_output);
//    free(ret_state);
//    
//    return NULL;
//}
//
//rpc_state* on_accept(rpc_state* state) {
//    msg_accept_t *msg_accp;
//    msg_accp = mpaxos__msg_accept__unpack(NULL, state->sz_input, state->raw_input);
//    //log_message_rid("receive", "ACCEPT", msg_accp->h, msg_accp->prop->rids,
//    //        msg_accp->prop->n_rids, state->sz);
//    rpc_state* ret_state = handle_msg_accept(msg_accp);
//    mpaxos__msg_accept__free_unpacked(msg_accp, NULL);
//
//    state->raw_output = (uint8_t*) malloc(ret_state->sz_output);
//    state->sz_output = ret_state->sz_output;
//    memcpy(state->raw_output, ret_state->raw_output, ret_state->sz_output);
//    free(ret_state->raw_output);
//    free(ret_state);
//
//    return NULL;
//}

//rpc_state* on_promise(rpc_state* state) {
//    msg_promise_t *msg_prom;
//    msg_prom = mpaxos__msg_promise__unpack(NULL, state->sz_input, state->raw_input);
//    LOG_DEBUG("receive PROMISE message");
//    //log_message_res("receive", "PROMISE", msg_prom->h, msg_prom->ress, 
//    //        msg_prom->n_ress, state->sz);
//    handle_msg_promise(msg_prom);
//    mpaxos__msg_promise__free_unpacked(msg_prom, NULL);
//    return NULL;
//}
//
//rpc_state* on_accepted(rpc_state* state) {
//    msg_accepted_t *msg_accd;
//    msg_accd = mpaxos__msg_accepted__unpack(NULL, state->sz_input, state->raw_input);
//    LOG_DEBUG("receive ACCEPTED message");
//    //log_message_res("receive", "ACCEPTED", msg_accd->h, msg_accd->ress, 
//    //        msg_accd->n_ress, state->sz);
//    handle_msg_accepted(msg_accd);
//    mpaxos__msg_accepted__free_unpacked(msg_accd, NULL);
//    LOG_DEBUG("finish handling ACCEPTED message");
//    return NULL;
//}
//
//rpc_state* on_decide(rpc_state* state) {
//    msg_decide_t *msg_dcd = NULL;
//    msg_dcd = mpaxos__msg_decide__unpack(NULL, state->sz_input, state->raw_input);
//    handle_msg_decide(msg_dcd);
//    mpaxos__msg_decide__free_unpacked(msg_dcd, NULL);
//    return NULL;
//}

void start_server(int port) {
    server_create(&server_, NULL);
    server_->comm->port = port;

    // register function
    //    server_reg(server_, RPC_PREPARE, on_prepare);
    //server_reg(server_, RPC_ACCEPT, on_accept);
    //server_reg(server_, RPC_DECIDE, on_decide);
    
    server_start(server_);
    LOG_INFO("Server started on port %d.", port);
    
    connect_all_senders();
}

void set_nid_sender(nodeid_t nid, char* addr, int port) {
    //Test save the key
    nodeid_t *nid_ptr = (nodeid_t *)apr_pcalloc(mp_global_, sizeof(nid));
    *nid_ptr = nid;
    client_t *c = NULL;
    client_create(&c, NULL);
    strcpy(c->comm->ip, addr);
    c->comm->port = port;
    // FIXME register function callbacks. 
    //    client_reg(c, RPC_PREPARE, on_promise);
    //client_reg(c, RPC_ACCEPT, on_accepted);

    SAFE_ASSERT(c != NULL);
    LOG_TRACE("setup client for node: %d", (int32_t) nid);
    apr_hash_set(ht_sender_, nid_ptr, sizeof(nid), c);
}
