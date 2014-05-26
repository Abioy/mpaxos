
#include "polling.h"
#include "rpc_comm.h"

#ifndef CLIENT_H
#define CLIENT_H

typedef struct {
    rpc_comm_t *comm;
    poll_job_t *pjob;
    buf_t *buf_recv;
    buf_t *buf_send;
} client_t;

void client_create(client_t **cli, poll_mgr_t *mgr);

void client_destroy(client_t *cli);

void client_connect(client_t *cli);

void client_disconnect(client_t *cli);

void handle_client_read(void *arg);

void handle_client_write(void *arg);

void client_reg(client_t *cli, msgid_t, void*);

#define client_regfun client_reg

#endif // CLIENT_H
