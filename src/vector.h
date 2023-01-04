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

#include <stdlib.h>


#define NFTP_SIZE         32
#define NFTP_BLOCK_SZ     (256 * 1024) // Maximal size of single package
#define NFTP_FILES        32 // Receive up to 32 files at once
#define NFTP_HASH(p, n)   nftp_djb_hashn(p, n)
#define NFTP_FNAME_LEN    64
#define NFTP_FDIR_LEN     256

enum NFTP_ERR {
	NFTP_ERR_HASH = 0x01,
	NFTP_ERR_FILEPATH,
	NFTP_ERR_FILENAME,
	NFTP_ERR_BLOCKS,
	NFTP_ERR_ID,
	NFTP_ERR_CONTENT,
	NFTP_ERR_FILE,
	NFTP_ERR_MEM,
	NFTP_ERR_OVERFLOW,
	NFTP_ERR_EMPTY,
	NFTP_ERR_PROTO,
	NFTP_ERR_DIRTY,
	NFTP_ERR_VEC,
	NFTP_ERR_IOVS,
	NFTP_ERR_FLAG,
	NFTP_ERR_STREAM,
	NFTP_ERR_HT,
	NFTP_ERR_TYPE,
};

#define NFTP_HEAD (-1)
#define NFTP_TAIL (0x7FFFFFFF)

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
