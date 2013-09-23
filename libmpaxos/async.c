
/**
 * Take the unthoughtful approach, each group
 * has a request queue and a lock. The deamon
 * keeps scanning every queue with the lock.
 */
#define MAX_THREADS 5

#include <apr_atomic.h>
#include <apr_time.h>
#include <apr_hash.h>
#include <apr_ring.h>
#include <apr_thread_proc.h>
#include <apr_thread_pool.h>
#include "mpaxos/mpaxos.h"
#include "async.h"
#include "view.h"
#include "utils/logger.h"
#include "utils/safe_assert.h"
#include "utils/mpr_dag.h"
#include "internal_types.h"
#include "slot_mgr.h"
#include "proposer.h"


static apr_pool_t *pl_async_; // a pool for constants in asynchronous callback module.
static mpaxos_cb_t cb_god_ = NULL; // a god callback function on all groups.
static apr_thread_pool_t *tp_async_;  // thread pool for asynchronous commit and call back.
static apr_thread_mutex_t *me_gids_; // a lock to help visit the three hash tables below.
static apr_hash_t *ht_me_cb_;   // groupid_t -> apr_thread_mutex_t, lock on each group for callback.
static apr_hash_t *ht_qu_cb_;   // groupid_t -> apr_ring_t, shit! finally I need a lot of queues to do this.
static apr_hash_t *ht_me_ri_;   // groupid_t -> apr_thread_mutex_t, lock on each group ring.
static apr_thread_t *th_daemon_; // daemon thread
static mpr_dag_t *dag_;


apr_uint32_t n_req_ = 0;
apr_uint32_t is_exit_ = 0;


typedef struct _req_elem_t {
    APR_RING_ENTRY(_req_elem_t) link;
    mpaxos_req_t *req;
} req_elem_t;
typedef struct _req_ring_t req_ring_t;
APR_RING_HEAD(_req_ring_t, _req_elem_t);


// unused now.
static apr_hash_t *cb_ht_;         //groupid_t -> mpaxos_cb_t
static apr_hash_t *cb_req_ht_;    //instid_t -> cb_para #not implemented in this version. 
static uint32_t n_subthread_ = 0;  // count of asynchrous threads.
static pthread_mutex_t n_subthreads_mutex;  //mutex lock for above   


void mpaxos_async_init() {
    apr_pool_create(&pl_async_, NULL);
    apr_thread_pool_create(&tp_async_, MAX_THREADS, MAX_THREADS, pl_global_);
    apr_thread_mutex_create(&me_gids_, APR_THREAD_MUTEX_UNNESTED, pl_async_);
    ht_me_cb_ = apr_hash_make(pl_async_);
    ht_me_ri_ = apr_hash_make(pl_async_);
    ht_qu_cb_ = apr_hash_make(pl_async_);
    mpr_dag_create(&dag_);

    // insert all groups into the two hash tables.
    groupid_t *gids;
    size_t sz_gids;

    // start the background daemon for asynchrous commit. 
    apr_thread_create(&th_daemon_, NULL, mpaxos_async_daemon, NULL, pl_async_); 

    // unused now.
    //cb_ht_ = apr_hash_make(pool_ptr);
    //cb_req_ht_ = apr_hash_make(pool_ptr);
    //apr_queue_create(&request_q_, QUEUE_SIZE, pool_ptr);
}

void mpaxos_async_enlist(groupid_t *gids, size_t sz_gids, uint8_t *data, size_t sz_data, void* cb_para) {
    mpaxos_req_t *r = (mpaxos_req_t *)malloc(sizeof(mpaxos_req_t));
    r->sz_gids = sz_gids;
    r->gids = malloc(sz_gids * sizeof(groupid_t));
    r->sz_data = sz_data;
    r->data = malloc(sz_data);
    r->cb_para = cb_para;
    memcpy(r->gids, gids, sz_gids * sizeof(groupid_t));
    memcpy(r->data, data, sz_data);

    mpr_dag_push(dag_, gids, sz_gids, r);
    
    LOG_DEBUG("request %d enlisted.", apr_atomic_read32(&n_req_));
}

/**
 * !!!IMPORTANT!!!
 * this function is NOT entirely thread-safe. same gid cannot call concurrently.
 */
void invoke_callback(mpaxos_req_t* p_r) {
    // TODO [fix]
    // check_callback(p_r);
    
    // get the callback function
    // LOG_DEBUG("invoke callback on group %d", gid);
    // mpaxos_cb_t *cb = (cb_god_ != NULL) ? &cb_god_ : apr_hash_get(cb_ht_, p_r->gids, sizeof(gid));
    SAFE_ASSERT(cb_god_ != NULL);
    
    (cb_god_)(p_r->gids, p_r->sz_gids, p_r->sids, p_r->data, p_r->sz_data, p_r->cb_para);
    // lock gid here.
    // check call history, and go forward. 
    // if there is value at sid + 1, then callback!
    // LOG_DEBUG("%d callback invoked on group %d, last callback: %d", c, gid, sid-1);
}

void mpaxos_set_cb(groupid_t gid, mpaxos_cb_t cb) {
    //groupid_t *k = apr_palloc(pool_ptr, sizeof(gid));
    //mpaxos_cb_t *v = apr_palloc(pool_ptr, sizeof(cb));
    //*k = gid;
    //*v = cb;
    //apr_hash_set(cb_ht_, k, sizeof(gid), v);

    //// initialize mutex for call back.
    //pthread_mutex_t *p_me = apr_palloc(pool_ptr, sizeof(pthread_mutex_t));
    //SAFE_ASSERT(p_me != NULL);
    //pthread_mutex_init(p_me, NULL);
    //apr_hash_set(cb_me_ht_, k, sizeof(groupid_t), p_me); 
}

void mpaxos_set_cb_god(mpaxos_cb_t cb) {
    cb_god_ = cb;
}

void mpaxos_async_destroy() {
    LOG_DEBUG("async module to be destroied.");
    // TODO [improve] recycle everything.
    apr_atomic_set32(&is_exit_, 1);
    apr_status_t s;
    mpr_dag_destroy(dag_);
    apr_thread_join(&s, th_daemon_);
    apr_thread_pool_destroy(tp_async_);
    apr_pool_destroy(pl_async_);
    LOG_DEBUG("async module destroied.");
}

void* APR_THREAD_FUNC async_commit_job(apr_thread_t *th, void *v) {
    // cannot call on same group concurrently, otherwise would be wrong.
    mpaxos_req_t *req = v;
    // commit_sync(req->gids, req->sz_gids, req->data, req->sz_data); 
  
    // find the appropriate proposer
    // keep propose until success
    LOG_DEBUG("try to commit asynchronously.");

    roundid_t **rids;
    rids = (roundid_t **) malloc(req->sz_gids * sizeof(roundid_t *));
    for (int i = 0; i < req->sz_gids; i++) {
        rids[i] = (roundid_t *)malloc(sizeof(roundid_t));
        mpaxos__roundid_t__init(rids[i]);
        rids[i]->gid = (req->gids[i]);

        //get_insnum(gids[i], &sid_ptr);
        //*sid_ptr += 1;
        rids[i]->bid = (1);
        rids[i]->sid = acquire_slot(req->gids[i], get_local_nid());
    }

    int ret = run_full_round_async(rids, req);

    //for (int i = 0; i < gid_len; i++) {
    //    free(rids_ptr[i]);
    //}
    //free(rids_ptr);
    return NULL;
}

void async_ready_callback(mpaxos_req_t *req) {
    // TODO [fix] for call back
	LOG_DEBUG("ready for call back.");
    (cb_god_)(req->gids, req->sz_gids, req->sids, req->data, req->sz_data, req->cb_para);
    void *data;
    mpr_dag_pop(dag_, req->gids, req->sz_gids, &data);
    SAFE_ASSERT(data == req);
}

/**
 * this runs in a background thread. keeps poping from the async job queue.
 * give the job the a thread in the thread pool.
 * its track should be like this. 
 *      1. background daemon thread keeps checking queues of asynchrous requests. 
 *      2. whenever it finds out there is a new request, it runs this request in one thread in pool.
 *      3. in each sub-thread, invoke_callback is called. 
 *      4. TODO [IMPROVE] one thread can loop do callback, other thread return. this is tricky because no tails should be left.
 */
void* APR_THREAD_FUNC mpaxos_async_daemon(apr_thread_t *th, void* data) {
    // suppose the thread is here
    LOG_DEBUG("async daemon thread start");
    while (!apr_atomic_read32(&is_exit_)) {
        // in a loop
        // check whether the pool or queue is empty?

        // use get all groups instead of iterating the hash table.
        // scan the hash table for queues.
        groupid_t *gids;
        size_t sz_gids;
        mpaxos_req_t *r;
        LOG_TRACE("getting white node from dag");
        apr_status_t status = mpr_dag_getwhite(dag_, &gids, &sz_gids, (void **)&r);
        
        if (status == APR_EOF) {
            LOG_DEBUG("daemon exiting.");
            break;
        }
        SAFE_ASSERT(status == APR_SUCCESS);
        LOG_TRACE("white node got from dag");
        apr_thread_pool_push(tp_async_, async_commit_job, (void*)r, 0, NULL);
    }
    LOG_DEBUG("async daemon thread exit");
    apr_thread_exit(th, APR_SUCCESS);
    return NULL;
}
