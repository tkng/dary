/*
  Dary - Double Array library
  Copyright (C) 2006-2007 TOKUNAGA Hiroyuki

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
*/

#ifndef _dary_h_included_
#define _dary_h_included_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdlib.h>

typedef struct _dary dary;

#ifndef uchar
typedef unsigned char uchar;
#endif

typedef enum _dary_status {
  DARY_STATUS_SUCCESS = 0,
  DARY_STATUS_FAIL,
  DARY_STATUS_NO_MEMORY,
  DARY_STATUS_NULL_POINTER,
  DARY_STATUS_READ_ERROR,
  DARY_STATUS_WRITE_ERROR,
  DARY_STATUS_FILE_NOT_FOUND,
} dary_status;


/* ! Note, API/ABI is UNSTABLE until release 1.0.0! */

/* New/free API */

//! creates a new dary object
/*!
 * dary_new creates a new dary object. returns null if fail.
 */
dary *dary_new(void);

dary *dary_new_from_file(const char *filename);
dary_status dary_write_to_file(dary *da, const uchar *filename);

void dary_free(dary *da);

/* Search API */
dary_status dary_exact_search(dary *da, const uchar *key, int *value);



/* Edit API */
dary_status dary_insert(dary *da, const uchar *key, int value);
dary_status dary_delete(dary *da, const uchar *key); /* Not implemented yet, always returns fail.*/

char *dary_get_key(dary *da, int pos);
int dary_get_value(dary *da, int pos);
dary_status dary_get_children(dary *da, int pos, int **children);


/* Misc API */
dary_status dary_get_base(dary *da, int **base);
dary_status dary_get_check(dary *da, int **check);
dary_status dary_get_values(dary *da, int **values);
dary_status dary_get_length(dary *da, int *len);

/* Debug API */
void dary_dump(const dary *da);
void dary_dump_range(const dary *da, int start, int end);


/* Utility */
typedef struct _dary_cb dary_cb;

dary_cb *dary_countbuffer_new(const char *filename);
int dary_countbuffer_append(dary_cb *cb, const char *str);
void dary_countbuffer_free(dary_cb *cb);

#ifdef __cplusplus
}
#endif
#endif /* _dary_h_included_ */
