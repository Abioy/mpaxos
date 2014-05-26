


#ifndef RPC_COMM_H
#define RPC_COMM_H

#include <stdint.h>
#include <apr_hash.h>
#include <apr_network_io.h>

#include "utils/mpr_hash.h"

#define SZ_MSGID sizeof(msgid_t)
#define SZ_SZMSG sizeof(uint32_t)

typedef uint16_t msgid_t;
typedef uint16_t funid_t;

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
} rpc_comm_t;

static void rpc_common_create(rpc_comm_t **comm) {
    *comm = (rpc_comm_t*) malloc(sizeof(rpc_comm_t));
    rpc_comm_t *c = *comm;
    apr_pool_create(&c->mp, NULL);
    apr_thread_mutex_create(&c->mx, APR_THREAD_MUTEX_UNNESTED, c->mp);
    mpr_hash_create_ex(&c->ht, 0);
    memset(c->ip, 0, 100);
}

static void rpc_common_destroy(rpc_comm_t *comm) {
    apr_thread_mutex_destroy(comm->mx);
    apr_pool_destroy(comm->mp);
}

#endif // RPC_COMM_H
