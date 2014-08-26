/*
 *
 * Author: loli
 * Date: 2014/8/26
 * Usage:Add/Remove request into/from a list  
 * PS: I love u, ms.
 *
 */

#include "include_all.h"
#include <mutex>
#include <deque>
static std::mutex req_mutex_;
static std::deque<mpaxos_req_t *> req_deque_;
static mpaxos_cb_t cb_god_ = NULL; // a god callback function on all groups.

void mpaxos_set_cb_god(mpaxos_cb_t cb) {
    cb_god_ = cb;
}

void mpaxos_enlist(mpaxos_req_t *req) {
	req_mutex_.lock();
	req_deque_.push_back(req);
	printf("sz: %d\n",req->sz_data);
	req_mutex_.unlock();
}

void mpaxos_retire(mpaxos_req_t *req) {
	req_mutex_.lock();
	if (req_deque_.empty() == false) {
		
		LOG_DEBUG("ready for call back.");
		printf("ready for call back.\n");

		(cb_god_)(req);

		void *data = NULL;
		data = req_deque_.front();
		req->tm_end = apr_time_now();
		SAFE_ASSERT(data == req);

		printf("sz: %d\n",req->sz_data);
		req_deque_.pop_front();
		printf("pop OK.\n");
		LOG_DEBUG("a instance finish. start:%ld, end:%ld", req->tm_start, req->tm_end);
		LOG_DEBUG("callback para: %x", req->cb_para);
    
		printf("sz2: %d\n",req->sz_data);
		// free req
	//	free(req->sids);
	//	free(req->gids);
//		free(req->data);
//	    free(req);
		delete req;
	}
	req_mutex_.unlock();
}
