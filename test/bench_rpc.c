/*
 * bench_rpc.c
 * Created on: Dec 2, 2013
 * Author: shuai
 */

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <apr_thread_cond.h>
#include <apr_atomic.h>
#include <apr_time.h>
#include "rpc/rpc.h"
#include "mpaxos.pb-c.h"

static apr_pool_t *mp_rpc_ = NULL;
static apr_thread_cond_t *cd_rpc_ = NULL;
static apr_thread_mutex_t *mx_rpc_ = NULL;

static char* addr_ = NULL;
static int port_ = 0;
static uint32_t n_client_ = 1;
static volatile apr_uint32_t n_rpc_ = 0;
static volatile apr_uint32_t n_svr_rpc_ = 0;
static bool is_server_ = false;
static bool is_client_ = false;
static int32_t max_rpc_ = 10000;

funid_t ADD = 1;
funid_t PROTO = 2;
static apr_time_t tm_begin_ = 0;
static apr_time_t tm_end_ = 0;

typedef struct {
    uint32_t a;
    uint32_t b;
} struct_add;

rpc_state* add(rpc_state *state) {
    uint32_t k = apr_atomic_add32(&n_svr_rpc_, 1);
    LOG_DEBUG("server add called. no: %d", k+1);
    struct_add *sa = (struct_add *)state->raw_input;
    uint32_t c = sa->a + sa->b;
    
    state->raw_output = (uint8_t*)malloc(sizeof(uint32_t));
    state->sz_output = sizeof(uint32_t);
    memcpy(state->raw_output, &c, sizeof(uint32_t));
    return NULL;
}

rpc_state* add_cb(rpc_state *state) {
    // Do nothing
    
    uint32_t j = apr_atomic_add32(&n_rpc_, 1);
    LOG_DEBUG("client callback exceuted. rpc no: %d", j+1);

    if (j + 1 == max_rpc_ * n_client_) {
        printf("hahaha\n");
        tm_end_ = apr_time_now();
        apr_thread_mutex_lock(mx_rpc_);
        apr_thread_cond_signal(cd_rpc_);
        apr_thread_mutex_unlock(mx_rpc_);
    }
    //    }
    return NULL;
}

//static rpc_state* on_accept() {
//
//}

void bench_proto() {
//    for (int i = 0; i < N_RPC; i++) {
//        // keep sending accept    
//        msg_accept_t msg_accp = MPAXOS__MSG_ACCEPT__INIT;
//        msg_header_t header = MPAXOS__MSG_HEADER__INIT;
//        msg_accp.h = &header;
//        msg_accp.h->t = MPAXOS__MSG_HEADER__MSGTYPE_T__ACCEPT;
//        msg_accp.h->tid = 0;
//        msg_accp.h->nid = 0;
//        proposal_t prop = MPAXOS__PROPOSAL__INIT;
//        msg_accp.prop = &prop;
//
//        size_t sz_msg = mpaxos__msg_accept__get_packed_size (&msg_accp);
//        char *buf = (char *)malloc(sz_msg);
//        mpaxos__msg_accept__pack(&msg_accp, (uint8_t *)buf);
//
//        free(buf);
//        client_call(client, PROTO, (uint8_t *)buf, sz_msg);
//    }
}

void bench_add() {
}

void* APR_THREAD_FUNC client_thread(apr_thread_t *th, void *v) {
    client_t *client = NULL;
    client_create(&client, NULL);
    strcpy(client->comm->ip, addr_);
    client->comm->port = port_;
    client_reg(client, ADD, add_cb);
    tm_begin_ = apr_time_now();
    client_connect(client);
//    printf("client connected.\n");
    for (int i = 0; i < max_rpc_; i++) {
        struct_add sa;
        sa.a = 1;
        sa.b = 2;
        client_call(client, ADD, (uint8_t *)&sa, sizeof(struct_add));
    }
    apr_thread_exit(th, APR_SUCCESS);
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

void usage(char *argv) {
    //    printf("Usage: %s server|client addr port\n ", argv);
    fprintf(stderr, "usage:%s ", argv);
    fprintf(stderr, "-(s|c), -a address -p port [-t n_ct]\n");
    fprintf(stderr, "where:\n");
    fprintf(stderr, "\t-s run as server\n");
    fprintf(stderr, "\t-c run as client\n");
    fprintf(stderr, "\t-a server address\n");
    fprintf(stderr, "\t-p server port\n");
    fprintf(stderr, "\t-t number of client threads\n");
    fprintf(stderr, "\t-m max number of rpc\n");
}

void arg_parse(int argc, char **argv) {
    char ch;
    while ((ch = getopt(argc, argv, "sca:p:t:hm:")) != EOF) {
	switch (ch) {
	case 's':
	    is_server_ = true;
	    break;
	case 'c':
	    is_client_ = true;
	    break;
	case 'a':
	    addr_ = optarg;
	    break;
	case 'p':
	    port_ = atoi(optarg);
	    break;
	case 't':
	    n_client_ = atoi(optarg);
	    break;
	case 'h':
	    usage(argv[0]);
	    exit(0);
	case 'm':
	    max_rpc_ = atoi(optarg);
	    break;
//	default:
//	    usage(argv[0]);
//	    return 0;
	}
    }
}

int main(int argc, char **argv) {
//    for (int i=1; i<NSIG; i++) {
//        //if (signal(SIGINT, sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");
//        if (signal(i, sig_handler) == SIG_ERR) printf("\ncan't catch %s\n", strsignal(i));
//    }
    signal(SIGPIPE, SIG_IGN);

    apr_initialize();
    apr_pool_create(&mp_rpc_, NULL);
    apr_thread_cond_create(&cd_rpc_, mp_rpc_);
    apr_thread_mutex_create(&mx_rpc_, APR_THREAD_MUTEX_UNNESTED, mp_rpc_);
    rpc_init();

    arg_parse(argc, argv);

    bool exit = false;

    exit |=  !(is_server_ || is_client_);
    exit |= (addr_ == NULL);
    exit |= (port_ == 0);

    if (exit) {
	fprintf(stderr, "wrong arguments.\n");
	usage(argv[0]);
	return 0;
    }

    if (is_server_) {
        server_t *server = NULL;
        server_create(&server, NULL);    
        strcpy(server->comm->ip, addr_);
        server->comm->port = port_;
        server_reg(server, ADD, add); 
        server_start(server);
        printf("server started.\n");
    } 
    
    if (is_client_) {
       
        apr_thread_mutex_lock(mx_rpc_);
        for (int i = 0; i < n_client_; i++) {
            apr_thread_t *th;
            apr_thread_create(&th, NULL, client_thread, NULL, mp_rpc_);
        }
        printf("rpc triggered for %d adds on %d threads.\n", max_rpc_ * n_client_, n_client_);
        apr_thread_cond_wait(cd_rpc_, mx_rpc_);
        apr_thread_mutex_unlock(mx_rpc_);

        int period = (tm_end_ - tm_begin_) / 1000;
        printf("finish %d rpc in %d ms.\n", max_rpc_ * n_client_, period);
        float rate = (float) max_rpc_ * n_client_ / 1000 / period;
        printf("rpc rate %f million per sec.\n", rate);
    }

    fflush(stdout);
    fflush(stderr);

    while(is_server_) {
	apr_sleep(1000000);
    }

    rpc_destroy();
    atexit(apr_terminate);
}

