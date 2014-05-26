#include "buf.h"

void buf_create(buf_t **buf) {
    *buf = (buf_t*) malloc(sizeof(buf_t));
    buf_t *b = *buf;
    b->raw = (uint8_t*) malloc(SZ_BUF);
    b->sz = SZ_BUF;
    b->idx_read = 0;
    b->idx_write = 0;
}

void buf_destory(buf_t *buf) {
    free(buf->raw);
    free(buf);
}

/*
 * make sure tail is larger than sz
 */
size_t buf_readjust(buf_t *buf, size_t sz) {
    while (buf->sz - buf->idx_write < sz) {
	if (buf->idx_read < buf->sz / 2) {
	    buf_realloc(buf, buf->sz * 2);
	} else {
	    buf_realloc(buf, buf->sz);
	}
    }
    return buf->sz;
}

/*
 * realloc the buf total size to sz
 */
size_t buf_realloc(buf_t *buf, size_t sz) {
    SAFE_ASSERT(sz > (buf->idx_write - buf->idx_read));
    // XXX make sure malloc safe
    uint8_t *nraw = (uint8_t*) malloc(sz);
    memcpy(nraw, buf->raw + buf->idx_read, buf->idx_write - buf->idx_read);
    free(buf->raw);
    buf->raw = nraw;
    buf->idx_write = buf->idx_write - buf->idx_read;
    buf->idx_read = 0;
    buf->sz = sz;
    return buf->sz;
}

size_t buf_write(buf_t *buf, uint8_t *data, size_t n) {
    buf_readjust(buf, n);
    memcpy(buf->raw + buf->idx_write, data, n);
    return n;
}

size_t buf_read(buf_t *buf, uint8_t *data, size_t n) {
    size_t sz_content = buf->idx_write - buf->idx_read;
    if (sz_content >= n) {
	memcpy(data, buf->raw + buf->idx_read, n);
	buf->idx_read += n;
	return n;
    } else {
	memcpy(data, buf->raw + buf->idx_read, sz_content);
	buf->idx_read += sz_content;
	return sz_content;
    }
}

size_t buf_peek(buf_t *buf, uint8_t *data, size_t n) {
    size_t sz_content = buf->idx_write - buf->idx_read;
    if (sz_content >= n) {
	memcpy(data, buf->raw + buf->idx_read, n);
	return n;
    } else {
	memcpy(data, buf->raw + buf->idx_read, sz_content);
	return sz_content;
    }
}

size_t buf_to_sock(buf_t *buf, apr_socket_t *sock) {
    apr_status_t status = APR_SUCCESS;
    
    while (true) {
	size_t n = buf->idx_write - buf->idx_read;
	if (n == 0) {
	    break;
	}
	status = apr_socket_send(sock, buf->raw + buf->idx_read, &n);
	SAFE_ASSERT(status == APR_SUCCESS);
	buf->idx_read += n;
    }
}

/**
 * 
 */
size_t buf_from_sock(buf_t *buf, apr_socket_t *sock) {
    apr_status_t status = APR_SUCCESS;

    while (true) {
	size_t empty_tail = buf->sz - buf->idx_write;
	size_t n = empty_tail;

	buf_readjust(buf, 1);
	status = apr_socket_recv(sock, buf->raw + buf->idx_write, &n);

	SAFE_ASSERT(n >= 0);
	SAFE_ASSERT(status == APR_SUCCESS);

	if (n == 0) {
	    // nothing more to read.
	    break;
	}

	buf->idx_read += n;
    }
}


