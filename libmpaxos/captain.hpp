
#pragma once

// TODO: [Loli] This module is responsible for locating the correct proposer and acceptor for a certain instance (an instance is identified by roundid). Do whatever you need to do.

// TODO proposer_t and acceptor_t: TBD.
proposer_t* mpaxos_find_proposer(XXX);
acceptor_t* mpaxos_find_acceptor(XXX);

// TODO: [Loli] start this request. 
int mpaxos_start_req(mpaxos_req_t *req);

// TODO: [Loli] this is committed successfully,
// notify the draft by mpaxos_retire.
int mpaxos_req_done(mpaxos_req_t *req);

// TODO: [Loli] commit failed, to retry.
int mpaxos_req_failed(mpaxos_req_t *req);


// TODO: [Loli] unmarshal the message, and dispatch
// to the correct proposer/acceptor to handle.
// refer to the commo.ccp -> on_accept, on_promise 
int mpaxos_msg_dispatch(rpc_state* state);


