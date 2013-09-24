

#include "utils/logger.h"
#include "internal_types.h"

static void log_message_rid(const char *action, const char *type,
        msg_header_t *h, roundid_t **rids, size_t sz_rids, size_t sz_msg) {
    char *log_buf = (char*)calloc(1000, sizeof(char));
    sprintf(log_buf, "%s %s message. nid: %lu, size: %lu", action, type, h->pid->nid, sz_msg);

//    int i;
//    for (i = 0; i < sz_rids; i++) {
//        const roundid_t *rid = rids[i];
//        sprintf(log_buf, "%s [gid:%"PRId64", sid:%"PRId64", bid:%"PRId64"]", log_buf,
//                rid->gid, rid->sid, rid->bid);
//    }
    LOG_DEBUG(log_buf);

    free(log_buf);
}

static void log_message_res(const char *action, const char *type,
        msg_header_t *h, Mpaxos__ResponseT **ress, int ress_len, size_t sz_msg) {
    char *log_buf = (char*)calloc(1000, sizeof(char));
    sprintf(log_buf, "%s %s message. nid: %lu, size: %lu", action, type, h->pid->nid, sz_msg);

//    int i;
//    for (i = 0; i < ress_len; i++) {
//        const Mpaxos__RoundidT *rid_p = ress[i]->rid;
//        sprintf(log_buf, "%s [gid:%"PRId64", sid:%"PRId64", bid:%"PRId64"]", log_buf,
//                rid_p->gid, rid_p->sid, rid_p->bid);
//    }
    LOG_DEBUG(log_buf);

    free(log_buf);
}
