#include "polling.h"

void* APR_THREAD_FUNC poll_worker_run(
        apr_thread_t *th,
        void *arg) {
    poll_worker_t *worker = arg;

    while (worker->fg_exit) {
        int num = 0;
        const apr_pollfd_t *ret_pfd;
        apr_status_t status = apr_pollset_poll(worker->ps, 100 * 1000, 
                &num, &ret_pfd);

        if (status == APR_SUCCESS) {
            SAFE_ASSERT(num > 0);
            for (int i = 0; i < num; i++) {
                int rtne = ret_pfd[i].rtnevents;
                SAFE_ASSERT(rtne & (APR_POLLIN | APR_POLLOUT));
                poll_job_t *job = ret_pfd[i].client_data;
                SAFE_ASSERT(job != NULL);
                if (rtne & APR_POLLIN) {
                    job->do_read(job->holder); 
                }
                if (rtne & APR_POLLOUT) {
                    job->do_write(job->holder);
                }
            }
        } else if (status == APR_EINTR) {
            // the signal we get when process exit, 
            // wakeup, or add in and write.
            LOG_WARN("the receiver epoll exits?");
            continue;
        } else if (status == APR_TIMEUP) {
            continue;
        } else {
            LOG_ERROR("unknown poll error. %s", 
                    apr_strerror(status, calloc(1, 100), 100));
            SAFE_ASSERT(0);
        }
    }
    
    SAFE_ASSERT(apr_thread_exit(th, APR_SUCCESS) == APR_SUCCESS);
    return NULL;
}

int poll_worker_start(poll_worker_t *worker) {
    apr_thread_create(&worker->th_poll, NULL, 
            poll_worker_run, (void*)worker, worker->mp_poll);  
    return 0;
}

int poll_worker_stop(poll_worker_t *worker) {
    apr_status_t status = 0;
    apr_thread_join(&status, worker->th_poll);
    return 0;
}

int poll_worker_create(
        poll_worker_t **worker,
        poll_mgr_t *mgr) {
    poll_worker_t *w = (poll_worker_t*) malloc(sizeof(poll_worker_t));
    w->mgr = mgr;
    apr_pool_create(&w->mp_poll, NULL);
    w->ht_jobs = apr_hash_make(w->mp_poll);
    apr_thread_mutex_create(&w->mx_poll, APR_THREAD_MUTEX_UNNESTED, w->mp_poll);
    poll_worker_start(w);
    *worker = w;
    return 0;
}

int poll_worker_destroy(
        poll_worker_t *worker) {
    worker->fg_exit = true; 
    poll_worker_stop(worker);    
    apr_pool_destroy(worker->mp_poll);
    free(worker);
    return 0;
}


int poll_worker_add_job(
        poll_worker_t *worker,
        poll_job_t *job) {
    apr_pollset_add(worker->ps, &job->pfd);  
}

int poll_worker_remove_job(
        poll_worker_t *worker,
        poll_job_t *job) {
    apr_pollset_remove(worker->ps, &job->pfd);
}

int poll_worker_update_job(
        poll_worker_t *worker,
        poll_job_t *job,
        int mode) {
    apr_pollset_remove(worker->ps, &job->pfd);   
    job->pfd.reqevents = mode;
    apr_pollset_add(worker->ps, &job->pfd);
}

int poll_mgr_create(
        poll_mgr_t **mgr,
        int n_worker) {
    poll_mgr_t *m;
    m = (poll_mgr_t*) malloc(sizeof(poll_mgr_t));
    m->n_worker = n_worker;
    m->workers = (poll_worker_t**) malloc(n_worker * sizeof(poll_mgr_t*)); 
    for (int i = 0; i < n_worker; i++) {
        poll_worker_create(&m->workers[i], m);
    }
    *mgr = m; 
    return 0;
}

int poll_mgr_destroy(
        poll_mgr_t *mgr) {
    for (int i = 0; i < mgr->n_worker; i++) {
        poll_worker_destroy(mgr->workers[i]);
    }
    free(mgr);    
    return 0;
}

int poll_mgr_add_job(
        poll_mgr_t *mgr,
        poll_job_t *job) {
    int i = 0; // TODO, hash
    poll_worker_t *worker = mgr->workers[i]; 
    poll_worker_add_job(worker, job);
    return 0;
}

int poll_mgr_remove_job(
        poll_mgr_t *mgr,
        poll_job_t *job) {
    int i = 0; // TODO, hash
    poll_worker_t *worker = mgr->workers[i]; 
    poll_worker_remove_job(worker, job);
    return 0;
}

int poll_mgr_update_job(
        poll_mgr_t *mgr,
        poll_job_t *job,
        int mode) {
    int i = 0; // TODO, hash
    poll_worker_t *worker = mgr->workers[i]; 
    poll_worker_update_job(worker, job, mode);
    return 0;
}
