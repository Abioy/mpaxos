
#ifndef RPC_COMM_H
#define RPC_COMM_H

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

#endif // RPC_COMM_H
