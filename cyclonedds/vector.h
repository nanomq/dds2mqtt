// Author: wangha <wangha at emqx dot io>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//
// It is a vector implementation from NFTP project by wangha.
//

#ifndef DDS2MQTT_VECTOR
#define DDS2MQTT_VECTOR

typedef struct handle handle;
struct handle {
	int    type; // 1->send 2->recv
	void * data;
	int    len;
};

typedef struct _vec nftp_vec;

int nftp_vec_alloc(nftp_vec **);
int nftp_vec_free(nftp_vec *);
int nftp_vec_append(nftp_vec *, void *);
int nftp_vec_insert(nftp_vec *, void *, int);
int nftp_vec_delete(nftp_vec *, void **, int);
int nftp_vec_push(nftp_vec *, void *, int);
int nftp_vec_pop(nftp_vec *, void **, int);
int nftp_vec_get(nftp_vec *, int, void **);
int nftp_vec_getidx(nftp_vec *, void *, int*);
int nftp_vec_cat(nftp_vec *, nftp_vec *);
size_t nftp_vec_cap(nftp_vec *);
size_t nftp_vec_len(nftp_vec *);

#endif
