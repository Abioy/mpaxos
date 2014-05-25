#ifndef RPC_H
#define RPC_H

#include <netinet/in.h>
#include <apr_thread_proc.h>
#include <apr_thread_pool.h>
#include <apr_poll.h>
#include "utils/mpr_hash.h"

//#include "internal_types.h"

#define BUF_SIZE__ (100 * 1024 * 1024)  // 100M

typedef uint8_t funid_t;

/*
 * this contains the shared information of server, client, 
 * and server_connection. the information includes the server
 * listen address and port (although useless in server_connection),
 * the function table to identify which function to call based
 * on the message type, and the socket information.
 */
typedef struct {
    char ip[100];       // server ip
    uint32_t port;       // server port
    apr_pool_t *mp;     // memory pool  
    apr_thread_mutex_t *mx;     // thread lock
    mpr_hash_t *ht;     // hash table of function call
    apr_sockaddr_t *sa; // socket_addr
    apr_socket_t *s;    // socket    
} rpc_common_t;

void rpc_common_create(rpc_comm_t **comm) {
    *comm = (rpc_comm_t*) malloc(sizeof(rpc_comm_t));
    rpc_comm_t *c = *comm;
    apr_pool_create(&c->mp, NULL);
    apr_thread_mutex_create(&c->mx, APR_THREAD_MUTEX_UNNESTED, c->mp);
    mpr_hash_create_ex(&c->comm->ht, 0);
    memset(c->ip, 0, 100);
}

void rpc_common_destroy(rpc_comm_t *comm) {
    apr_thread_mutex_destroy(comm->mx);
    apr_pool_destroy(comm->mp);
}

//typedef struct {
//    apr_socket_t *s;
//    //void* APR_THREAD_FUNC (*on_recv)(apr_thread_t *th, void* arg);
//
//    apr_pool_t *mp;
//    apr_thread_mutex_t *mx;
//    apr_pollset_t *ps;  
//    rpc_common_t *com;
//    uint64_t n_rpc;
//    uint64_t sz_recv;
//    uint64_t sz_send;
//    
//    struct  {
//        uint8_t *buf;
//        size_t sz;
//        size_t offset_begin;
//        size_t offset_end;
//    } buf_send;
//    
//    struct  {
//        uint8_t *buf;
//        size_t sz;
//        size_t offset_begin;
//        size_t offset_end;
//    } buf_recv;
//    
//    apr_pollfd_t pfd;
//} context_t; // This looks like a pollable to me.
//
//typedef struct {
//    rpc_common_t com;
//    context_t *ctx;
//} client_t;
//
//typedef struct {
//    rpc_common_t com;
//    context_t *ctx;
//    context_t **ctxs;
//    size_t sz_ctxs;
//} server_t;
//
typedef struct {
    uint8_t *raw_input;
    size_t sz_input;
    uint8_t *raw_output;
    size_t sz_output;
    sconn_t *sconn;
} rpc_state;



void rpc_init();

void rpc_destroy();

//void server_create(server_t **);
//
//void server_destroy(server_t *); 
//
//void server_regfun(server_t *, funid_t, void *);
//
//void server_start(server_t *);
//
//void server_stop(server_t *);
//
//void client_create(client_t **);
//
//void client_destroy(client_t *);
//
//void client_connect(client_t *);
//
//void client_disconnect(client_t *);
//
//void client_call(client_t *, funid_t, const uint8_t *, size_t);
//
//void client_regfun(client_t *, funid_t, void *);
//
//void client_regfun(client_t *, funid_t, void *);
//
//context_t *context_gen(rpc_common_t *);
//
/*===============================================================*/
/*                     legacy code below                         */ 
/*===============================================================*/

//
//typedef struct read_state {
////    struct event *ev_read;
//    uint8_t *data;
//    size_t  sz_data;
//    uint8_t *buf_write;
//    size_t sz_buf_write;
//    context_t *ctx;
//    funid_t reply_msg_type;
//} read_state_t;
//
//typedef struct {
//    uint8_t *data;
//    size_t  sz_data;
//    uint8_t *buf_write;
//    size_t sz_buf_write;
//    client_t *client;
//    apr_sockaddr_t remote_sa;
//    apr_socket_t *s;
//} write_state_t;
//
//

//void* APR_THREAD_FUNC start_poll(apr_thread_t *t, void* v);
//
//void reply_to(read_state_t *state);
//
//void add_write_buf_to_ctx(context_t *ctx, funid_t, const uint8_t *buf, 
//        size_t sz_buf);
//
//void context_destroy(context_t *ctx);
//
#endif
