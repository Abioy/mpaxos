#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <event.h>
#include <event2/thread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <apr_thread_proc.h>
#include <apr_network_io.h>
#include <apr_poll.h>
#include <errno.h>
#include "sendrecv.h"
#include "comm.h"
#include "utils/logger.h"
#include "utils/safe_assert.h"

#define MAX_THREADS 100
#define POLLSET_NUM 1000

extern apr_pool_t *pl_global_;
apr_thread_t *t;

apr_pollset_t *pollset_ = NULL;


static int exit_ = 0;

void init_recvr(recvr_t* r) {
    apr_status_t status;
    apr_pool_create(&r->pl_recv, NULL);
    apr_sockaddr_info_get(&r->sa, NULL, APR_INET, r->port, 0, r->pl_recv);
/*
    apr_socket_create(&r->s, r->sa->family, SOCK_DGRAM, APR_PROTO_UDP, r->pl_recv);
*/
    apr_socket_create(&r->s, r->sa->family, SOCK_STREAM, APR_PROTO_TCP, r->pl_recv);
    apr_socket_opt_set(r->s, APR_SO_NONBLOCK, 1);
    apr_socket_timeout_set(r->s, -1);
    apr_socket_opt_set(r->s, APR_SO_REUSEADDR, 1);/* this is useful for a server(socket listening) process */
    apr_socket_opt_set(r->s, APR_TCP_NODELAY, 1);
    
    status = apr_socket_bind(r->s, r->sa);
    if (status != APR_SUCCESS) {
        LOG_ERROR("cannot bind.");
        printf("%s", apr_strerror(status, malloc(100), 100));
        SAFE_ASSERT(status == APR_SUCCESS);
    }
    status = apr_socket_listen(r->s, 20);
    if (status != APR_SUCCESS) {
        LOG_ERROR("cannot listen.");
        printf("%s", apr_strerror(status, malloc(100), 100));
        SAFE_ASSERT(status == APR_SUCCESS);
    }
    r->buf_recv.buf = calloc(BUF_SIZE__, 1);
    r->buf_recv.sz = BUF_SIZE__;
    r->buf_recv.offset_begin = 0;
    r->buf_recv.offset_end = 0;
    
    r->buf_send.sz = BUF_SIZE__;
    r->buf_send.buf = calloc(BUF_SIZE__, 1);
    r->buf_send.offset_begin = 0;
    r->buf_send.offset_end = 0;
}

context_t *get_context() {
    context_t *ctx = malloc(sizeof(context_t));
    ctx->buf_recv.buf = malloc(BUF_SIZE__);
    ctx->buf_recv.sz = BUF_SIZE__;
    ctx->buf_recv.offset_begin = 0;
    ctx->buf_recv.offset_end = 0;
    
    ctx->buf_send.sz = BUF_SIZE__;
    ctx->buf_send.buf = malloc(BUF_SIZE__);
    ctx->buf_send.offset_begin = 0;
    ctx->buf_send.offset_end = 0;
    
    ctx->on_recv = on_recv;
   
    apr_pool_create(&ctx->mp, NULL);
    apr_thread_mutex_create(&ctx->mx, APR_THREAD_MUTEX_UNNESTED, ctx->mp);
    return ctx;
}

void add_write_buf_to_ctx(context_t *ctx, const uint8_t *buf, size_t sz_buf) {
    apr_thread_mutex_lock(ctx->mx);
    // realloc the write buf if not enough.
    if (sz_buf + sizeof(size_t) > ctx->buf_send.sz - ctx->buf_send.offset_end) {
        LOG_TRACE("remalloc sending buffer.");
        uint8_t *newbuf = malloc(BUF_SIZE__);
        memcpy(newbuf, ctx->buf_send.buf + ctx->buf_send.offset_begin, 
                ctx->buf_send.offset_end - ctx->buf_send.offset_begin);
        free(ctx->buf_send.buf);
        ctx->buf_send.buf = newbuf;
        ctx->buf_send.offset_end -= ctx->buf_send.offset_begin;
        ctx->buf_send.offset_begin = 0;
        SAFE_ASSERT(sz_buf + sizeof(size_t) < ctx->buf_send.sz - ctx->buf_send.offset_end);
    } else {
        SAFE_ASSERT(1);
    }
    // copy memory
    LOG_TRACE("add message to sending buffer, message size: %d", sz_buf);
     
    *(size_t*)(ctx->buf_send.buf + ctx->buf_send.offset_end) = sz_buf;
    LOG_TRACE("size in buf:%llx, original size:%llx", *(ctx->buf_send.buf + ctx->buf_send.offset_end), sz_buf);
    ctx->buf_send.offset_end += sizeof(size_t);
    memcpy(ctx->buf_send.buf + ctx->buf_send.offset_end, buf, sz_buf);
    ctx->buf_send.offset_end += sz_buf;
    
    // change poll type
    if (ctx->pfd.reqevents == APR_POLLIN) {
        apr_pollset_remove(pollset_, &ctx->pfd);
        ctx->pfd.reqevents = APR_POLLIN | APR_POLLOUT;
        apr_pollset_add(pollset_, &ctx->pfd);
    }
    
    apr_thread_mutex_unlock(ctx->mx);
}

void reply_to(read_state_t *state) {
    add_write_buf_to_ctx(state->ctx, state->buf_write, state->sz_buf_write);
    free(state->buf_write);
/*

    // add to pollset
    apr_socket_t *s = state->s;
    apr_pollfd_t pfd = {pl_global_, APR_POLL_SOCKET, APR_POLLOUT, 0, {NULL}, NULL};
    pfd.desc.s = state->s;
    pfd.client_data = state;
    apr_status_t status;
    status = apr_pollset_add(pollset_, &pfd);
    SAFE_ASSERT(status == APR_SUCCESS);
*/
    
    
}
void on_write(context_t *ctx, const apr_pollfd_t *pfd) {
    apr_thread_mutex_lock(ctx->mx);
    // LOG_DEBUG("write msg on socket %d", pfd->desc.s);
    apr_status_t status;
    uint8_t *buf = ctx->buf_send.buf + ctx->buf_send.offset_begin;
    size_t n = ctx->buf_send.offset_end - ctx->buf_send.offset_begin;
    if (n > 0) {
        status = apr_socket_send(pfd->desc.s, (char *)buf, &n);
        LOG_TRACE("sent data size: %d", n);
        SAFE_ASSERT(status == APR_SUCCESS);
        ctx->buf_send.offset_begin += n;
    } else {
        SAFE_ASSERT(0);
    }
    
    n = ctx->buf_send.offset_end - ctx->buf_send.offset_begin;
    if (n == 0) {
        // buf empty, remove out poll.
        apr_pollset_remove(pollset_, &ctx->pfd);
        ctx->pfd.reqevents = APR_POLLIN;
        apr_pollset_add(pollset_, &ctx->pfd);
    }
    
    apr_thread_mutex_unlock(ctx->mx);
}

// TODO [fix]
void on_read(context_t * ctx, const apr_pollfd_t *pfd) {
    // LOG_DEBUG("HERE I AM, ON_READ");
    
    apr_status_t status;

//    LOG_DEBUG("start reading socket");
    uint8_t *buf = ctx->buf_recv.buf + ctx->buf_recv.offset_end;
    size_t n = ctx->buf_recv.sz - ctx->buf_recv.offset_end;
    
    status = apr_socket_recv(pfd->desc.s, (char *)buf, &n);
//    LOG_DEBUG("finish reading socket.");
    if (status == APR_SUCCESS) {
        ctx->buf_recv.offset_end += n;
        if (n == 0) {
            LOG_WARN("received an empty message.");
        } else {
            // LOG_DEBUG("raw data received.");
            // extract message.
            while (ctx->buf_recv.offset_end - ctx->buf_recv.offset_begin > sizeof(size_t)) {
                size_t sz_msg = *((size_t *)(ctx->buf_recv.buf + ctx->buf_recv.offset_begin));
                LOG_TRACE("next recv message size: %d", sz_msg);
                if (ctx->buf_recv.offset_end - ctx->buf_recv.offset_begin >= sz_msg + sizeof(size_t)) {
                    buf = ctx->buf_recv.buf + ctx->buf_recv.offset_begin + sizeof(size_t);
                    struct read_state *state = malloc(sizeof(struct read_state));
                    state->sz_data = sz_msg;
                    state->data = malloc(state->sz_data);
                    memcpy(state->data, buf, sz_msg);
                    state->ctx = ctx;
                    ctx->buf_recv.offset_begin += sz_msg + sizeof(size_t);
                    (*(ctx->on_recv))(NULL, state);
                } else {
                    break;
                }
            }

            if (ctx->buf_recv.offset_end + BUF_SIZE__ / 10 > ctx->buf_recv.sz) {
                // remalloc the buffer
                LOG_TRACE("remalloc recv buf");
                uint8_t *buf = calloc(BUF_SIZE__, 1);
                memcpy(buf, ctx->buf_recv.buf + ctx->buf_recv.offset_begin, 
                        ctx->buf_recv.offset_end - ctx->buf_recv.offset_begin);
                free(ctx->buf_recv.buf);
                ctx->buf_recv.buf = buf;
                ctx->buf_recv.offset_end -= ctx->buf_recv.offset_begin;
                ctx->buf_recv.offset_begin = 0;
            }
        }
    } else if (status == APR_EOF) {
        LOG_WARN("received apr eof, what to do?");
        apr_pollset_remove(pollset_, &ctx->pfd);
    } else if (status == APR_ECONNRESET) {
        LOG_WARN("oops, seems that i just lost a buddy");
        apr_pollset_remove(pollset_, &ctx->pfd);
    } else {
        printf("%s", apr_strerror(status, malloc(100), 100));
        SAFE_ASSERT(0);
    }
        
//        apr_thread_t *t;
//        apr_thread_create(&t, NULL, r->on_recv, (void*)state, r->pl_recv);
//        apr_thread_pool_push(r->tp_recv, r->on_recv, (void*)state, 0, NULL);
        // send back the response in on_recv.
/*
    if (s != r->s) {
        apr_pollfd_t pfd = {pl_global_, APR_POLL_SOCKET, APR_POLLIN, 0, {NULL}, NULL};
        pfd.desc.s = s;
        apr_pollset_remove(pollset_, &pfd);
    }
*/
}

void on_accept(recvr_t *r) {
    apr_status_t status;
    apr_socket_t *ns;
    status = apr_socket_accept(&ns, r->s, pl_global_);
    SAFE_ASSERT(status == APR_SUCCESS);
    apr_socket_opt_set(ns, APR_SO_NONBLOCK, 1);
    apr_socket_opt_set(ns, APR_TCP_NODELAY, 1);
    context_t *ctx = get_context();
    ctx->s = ns;
    apr_pollfd_t pfd = {pl_global_, APR_POLL_SOCKET, APR_POLLIN, 0, {NULL}, NULL};
    ctx->pfd = pfd;
    ctx->pfd.desc.s = ns;
    ctx->pfd.client_data = ctx;
    apr_pollset_add(pollset_, &ctx->pfd);
}

void* APR_THREAD_FUNC run_recvr(apr_thread_t *t, void* arg) {
    recvr_t* r = arg;
    apr_pollset_create(&pollset_, POLLSET_NUM, r->pl_recv, APR_POLLSET_THREADSAFE);
    apr_pollfd_t pfd = {r->pl_recv, APR_POLL_SOCKET, APR_POLLIN, 0, {NULL}, NULL};
    pfd.desc.s = r->s;
    apr_pollset_add(pollset_, &pfd);
    
    apr_status_t status;
    while (!exit_) {
        int num;
        const apr_pollfd_t *ret_pfd;
        status = apr_pollset_poll(pollset_, -1, &num, &ret_pfd);
        if (status == APR_SUCCESS) {
            SAFE_ASSERT(num > 0);
            for(int i = 0; i < num; i++) {
                if (ret_pfd[i].rtnevents & APR_POLLIN) {
                    if(ret_pfd[i].desc.s == r->s) {
                        on_accept(r);
                    } else {
                        on_read(ret_pfd[i].client_data, &ret_pfd[i]);
                    }
                } 
                if (ret_pfd[i].rtnevents & APR_POLLOUT) {
                    on_write(ret_pfd[i].client_data, &ret_pfd[i]);
                }
                if (!(ret_pfd[i].rtnevents | APR_POLLOUT | APR_POLLIN)) {
                    // have no idea.
                    SAFE_ASSERT(0);
                }
            }
        } else if (status == APR_EINTR) {
            // the signal we get when process exit
            LOG_INFO("the receiver epoll exits.");
        } else {
            char buf[100];
            apr_strerror(status, buf, 100);
            LOG_DEBUG(buf);
            SAFE_ASSERT(0);
        }
    }
    apr_thread_exit(t, APR_SUCCESS);
    return NULL;
}

void stop_server() {
    if (t) {
        exit_ = 1;
        LOG_DEBUG("recv server ends.");
//        apr_thread_join(NULL, t);
    }
}

void run_recvr_pt(recvr_t* r) {
    apr_thread_create(&t, NULL, run_recvr, (void*)r, pl_global_);
}
