#include "include_all.h"

int mpaxos_msg_dispatch(rpc_state* state) {
}

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
//
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
