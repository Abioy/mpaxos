/*
 * proposer.h
 *
 * Created on: Jan 2, 2013
 *    Author: Shuai Mu
 *
 * proposer.hpp
 *    Author: Lijing Wang
 */

// TODO (loli) rename this to proposer.hpp, and make this compilable, and implement this Proposer class 

#pragma once

//#include <apr_hash.h>
//#include <pthread.h>
//
//#include "mpaxos/mpaxos-types.h"
//#include "internal_types.h"
//#include "utils/safe_assert.h"

class PropValue {
    // value_id_it should be 64-bit uint, 
    // high 16-bit is the prop_id, 
    // low 48-bit is a self-incremental counter;  
    value_id_t id_;
    std::string data_;
}

class Proposer {
public:
    View *view_; 

    // the value that client wants me to commit
    PropValue *init_value_; 

    // the ballot I am currently using
    ballot_it_t curr_ballot_; 
    
    // the value I am currently proposing
    PropValue *curr_value_;  
    
    // the max ballot I have ever seen, initialy 0;
    ballot_id_t max_ballot_;  

    // the max PropValue I have ever seen, initialy null.
    PropValue *max_value_;

    // the callback after I finish this instance.  
    std::function<void>(PropValue &value) callback_;
        
    Proposer(View &view, PropValue &value, std::function<void>(PropValue &value) &callback); 

    // the ack from all nodes to the current prepare.  
    // remember to clean for every next ballot!!!
    std::map<node_id_t, ack_prepare*> ack_prepare_;

    // the ack from all nodes to the cuurent accept. 
    // remember to clean for every next ballot!!!
    std::map<node_id_t, ack_accept*> ack_accept_;

    /**
     * ballot_id_t should be a 64-bit uint, high 48bit is self incremental
     * counter, low 16bit is the node id. 
     */
    ballot_id_t gen_next_ballot() ;

    // start proposing, send prepare requests to at least a majority of  
    // acceptors .
    void start();

    // choose a higher ballot id, retry the prepare phase.
    // clean the map for the ack of prepare and accept
    void restart();

    /**
     * handle acks to the prepare requests.
     * if a majority of yes, decide use which value to do the accept requests.
     *   with a majority of empty, use the initial value I have. 
     *   with some already accepted value, choose one with the highest ballot
     * if a majority of no, 
     *   restart with a higher ballot  
     */
    void handle_msg_promise(ack_prepare_t *);

    /**
     * handle acks to the accept reqeusts;
     * if a majority of yes, the value is successfully chosen. 
     * 
     * if a majority of no,
     *    restart the first phase with a higher ballot.
     */
    void handle_msg_accepted(ack_accept_t *);   
    
    ~Proposer(); 
}
