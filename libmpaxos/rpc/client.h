
#include "pollable.h"

#ifndef CLIENT_H
#define CLIENT_H

typedef struct {
    rpc_common_t comm;
    poll_job_t *pjob;
    buf_t *buf_recv;
    buf_t *buf_send;
} client_t;

void client_create(client_t **cli);

void client_destroy(client_t *cli);

void client_connect(client_t *cli);

void client_disconnect(client_t *cli);

void handle_client_read(void *arg);

void handle_client_write(void *arg);

void client_reg(client_t *cli, funid_t, void*);

#endif // CLIENT_H
