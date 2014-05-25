

#include "polling.h"


typedef struct {
    rpc_common_t *comm;
    poll_job_t *pjob;
    mpr_hash_t *ht_conn;
    bool is_start;
} server_t;

typedef struct {
    rpc_common_t *comm;
    poll_job_t *job;
    buf_t *buf_recv;
    buf_t *buf_send;
} sconn_t;

void server_create(server_t **svr);

void server_destroy(server_t *svr);

void server_start(server_t *svr);

void server_stop(server_t *svr);

void server_reg(server_t *svr, funid_t, void*);

void sconn_create(sconn_t **sconn);

void sconn_create(sconn_t *sconn);

void handle_server_read(void* arg);

void handle_sconn_read(void* arg);

void handle_sconn_write(void* arg);
