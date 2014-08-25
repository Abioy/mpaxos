/**
 *
 * Author:loli
 * Date:2014/8/23
 */
#include "rpc/rpc.h"

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

void sig_handler(int signo) {
    char *s = strsignal(signo);
    printf("received signal. type: %s\n", s);
    if (signo == SIGINT) {
        exit(0);
    } else if (signo == SIGABRT) {
        exit(0);
    } else {
        //do nothing.
    }
}

START_TEST (rpc) {
//int main(){
    //if (signal(SIGINT, sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");
    signal(SIGPIPE, SIG_IGN);

    server_t *server = NULL;
    client_t *client = NULL;
	poll_mgr_t *mgr = NULL;
//	poll_mgr_t *mgr_client = NULL;

	apr_initialize();

    rpc_init();
    poll_mgr_create(&mgr, 1);  
    server_create(&server, mgr);    
    strcpy(server->comm->ip, "127.0.0.1");
    server->comm->port = 9999;
    server_reg(server, ADD, (void*)add); 
    server_start(server);
    printf("server started.\n");

  //  poll_mgr_create(&mgr_client, 1);  
    client_create(&client, mgr);
    strcpy(client->comm->ip, "127.0.0.1");
    client->comm->port = 9999;
    client_reg(client, ADD, (void*)add_cb);
    client_connect(client);
    printf("client connected.\n");

    struct_add sa;
    sa.a = 1;
    sa.b = 2;
    client_call(client, ADD, (uint8_t *)&sa, sizeof(struct_add));
    printf("client called.\n");
    rpc_destroy();
}
END_TEST
