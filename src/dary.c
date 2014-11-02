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

#include "dary.h"

#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct _dary {
   int *base;
   int *check;
   int *values;
   int free_head; /* head of the list of unused nodes. */
   int length;
   int can_free;
};

#undef MIN
#undef MAX

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))


static int
find_free_node(dary *da, const uchar minimum);

static int
get_children_num(dary *da, int index);

static int
get_children(dary *da, int index, int *loc);

static inline int
is_free_node(dary *da, int index)
{
   if (da->length <= index) {
      dprint("is_free_node failed. length = %d but try to read %d\n", da->length, index);
      dary_dump(da);
      abort();
   }

   if (da->check[index] < 0)
      return 1;
   else
      return 0;
}


static inline int
is_leaf_node(dary *da, int index)
{
   if (da->base[index] == 0)
      return 1;
   else
      return 0;
}

static inline int
is_last_free_node(dary *da, int index)
{
   dprint("da->check[%d] = %d", index, da->check[index]);
   if (da->check[index] == -index)
      return 1;
   else
      return 0;
}

static inline int
is_no_conflict(dary *da, int m, int n, uchar k)
{
   /* argument k is need for future extention. */

   if (da->check[m] == n) {
      return 1;
   }
   return 0;
}

static inline int
get_parent_index(dary *da, int index)
{
   return da->check[index];
}

static int
find_last_free_node(dary *da)
{
   if (da->length == 0) {
      return -1;
   } else if (da->free_head < 0) {
      return -1;
   }

   int i = da->free_head;
   
   while (1) {
      if(i < 0) {
         dprint("find_last_free_node, something wrong occured.");
         dary_dump(da);
         abort();
      }
      //      dprint("i:%d, check[i] = %d", i, da->check[i]);
      if(da->check[i] == -i) {
         return i;
      }
      i = - da->check[i];
   }
  
   return -1;
}

static int
find_prev_free_node(dary *da, int index)
{
   int i;

   i = da->free_head;

   while (i > 0 && i < index) {
      //    printf("search_prev_free_node i:%d, index %d, check[%d]==%d\n",
      //           i, index, i, da->check[i]);
      if(- da->check[i] >= index) {
         return i;
      } else if (da->check[i] == -i) {
         return i;
      }
      i = - da->check[i];
   }
   return -1;
}


static void
remove_from_freelist(dary *da, int index)
{
   assert(is_free_node(da, index));
   
   dprint("remove %d from freelist", index);
   
   if (da->free_head == index) {
      if (is_last_free_node(da, index)) {
         da->free_head = -1;
      } else {
         da->free_head = -da->check[index];
      }
   } else {
      int prev_free_node = find_prev_free_node(da, index);
      if (is_last_free_node(da, index)) {
         da->check[prev_free_node] = - prev_free_node;
         dprint("remove_from_freelist1: check[%d] = %d", prev_free_node, - prev_free_node);
      } else {
         da->check[prev_free_node] = da->check[index];
         dprint("remove_from_freelist2: check[%d] = %d", prev_free_node, da->check[index]);
      }
   }
}


static void *(*dary_allocator)(size_t size) = malloc;

void dary_set_allocator(void *(*allocator)(size_t size))
{
   dary_allocator = allocator;
}

static dary_status
validate_array(dary *da)
{
   int i,j;

   for(i=1;i<da->length;i++) {
      for(j=i;j<da->length;j++) {
         if(da->check[j] == i && j > da->base[i]+255) {
            dprint("!Impossible transition exists from %d to %d (base[%d]=%d)", i, j, i, da->base[i]);
            abort();
         }
      }   
   }
  
   if(da->free_head > 0) {
      int next_empty  = da->free_head;

      for(i = 2; i < da->length; i++) {
         if(da->check[i] < 0) {
            if(i == next_empty) {
               if(da->check[-da->check[i]] > 0) {
                  dprint("Non empty node found in %d, i = %d", - da->check[i], i);
                  dary_dump(da);
                  abort();
               }
               next_empty = - da->check[i];
            } else if(next_empty > i) {
               dprint("Connecting error. %d is not connected from previous unused node", i);
               dary_dump(da);
               abort();
            }
         }
      }
   }

   return DARY_STATUS_SUCCESS;
}

char *
dary_get_key(dary *da, int pos)
{
   char *key;
   int i = pos;
   int len = 0;

#define DARY_START 0

   if(pos <= 1 || pos >= da->length) {
      return NULL;
   }

   if (is_free_node(da, pos)) {
      return NULL;
   } 

   do {
      i = da->check[i];
      len++;
   } while(i != DARY_START);
    
   key = malloc(len+1);

   if (!key) {
      fprintf(stderr, "Memory allocation failed!");
      
   }
  
   key[len] = '\0';
   len--;
   i = pos;

   do {
      key[len] = i - da->base[da->check[i]];
      i = da->check[i];
      len--;
   } while(i != DARY_START);
    
   return key;
}

int
dary_get_value(dary *da, int pos)
{
   if (da->length > pos) {
      return da->values[pos];      
   } else {
      return -1;
   }
}

int *
dary_realloc(int *p, int new_len, int old_len)
{
   int *new_ptr = (int*)dary_allocator(new_len);

   if(new_ptr) {
      int i = 0;
    
      for(i=0;i<old_len;i++) {
         new_ptr[i] = p[i];
      }
      return new_ptr;
   } else {
      return NULL;
   }
}


static dary_status
extend_array(dary *da, int new_length)
{
   int i;
   int old_length = da->length;
   int last_free_node = find_last_free_node(da);
   int *base, *check;
   int *values;
   //  printf("extend_array new array size:%d (old size = %d)\n", new_length, old_length);

   base = realloc(da->base,   new_length * sizeof (int));
   if(base == NULL) {
      return DARY_STATUS_NO_MEMORY;
   }
   da->base = base;

   check = realloc(da->check,  new_length * sizeof (int));
   if(check == NULL) {
      return DARY_STATUS_NO_MEMORY;
   }
   da->check = check;

   values = realloc(da->values,  new_length * sizeof (int));
   if(values == NULL) {
      return DARY_STATUS_NO_MEMORY;
   }
   da->values = values;

   for(i = old_length; i< new_length; i++) {
      da->base[i]  = 0;
      da->check[i] = - (i + 1);
      da->values[i] = 0;
   }

   da->check[new_length - 1] = - (new_length - 1);

   if(last_free_node > 0) {
      dprint("write to da->check[%d] = %d, new_length = %d", last_free_node, -old_length, new_length);
      da->check[last_free_node] = - old_length;
   } else {
      da->free_head = old_length;
   }

   da->length = new_length;

   return DARY_STATUS_SUCCESS;
}


dary *
dary_new(void)
{
   dary *da;
   int i = 0;
   da = dary_allocator(sizeof(dary));

   da->length = 0;
   da->base = NULL;
   da->check = NULL;
   da->values = NULL;
   da->free_head = -1;
   extend_array(da, UCHAR_MAX + 2);
   da->free_head = UCHAR_MAX + 1;
 
   for (i =0 ; i < UCHAR_MAX + 1; i++) {
      da->base[i] = 0;
      da->check[i] = 0;
   }
   dprint("%d",i);
   da->check[i] = -i;
  
   da->can_free = 1;
   return da;
}


dary *
dary_new_from_mem(int *base_start, int *check_start, int *values_start, int length, int can_free)
{
   dary *da;
   da = malloc(sizeof(dary));
  
   if (!da) {
      return NULL;
   }

   da->length = length;
   da->base = base_start;
   da->check = check_start;
   da->values = values_start;
   da->free_head = -1;
   da->can_free = can_free;
   return da;
}

dary *
dary_new_from_file(const char *filename)
{
   int fd;
   struct stat st;
   dary *da;  
   int length;
   int *base;
   int *check;
   int *values;

   fd = open(filename, O_RDONLY);
  
   if (fd == -1 || fstat(fd, &st) == -1)
      return NULL;
   
   if (st.st_size % 3 != 0) {
      close(fd);
      return NULL;
   }
    
   base = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
   close(fd);
  
   if (!base)
      return NULL;
  
   length = st.st_size / (sizeof(int) * 3);
  
   check  = base + length;
   values = base + length * 2;

   da = dary_new_from_mem(base, check, values, length, 0);
   return da;
}

dary_status
dary_write_to_file(dary *da, const uchar *filename)
{
   FILE *fp = fopen(filename, "w");

   if(!fp)
      return DARY_STATUS_WRITE_ERROR;
   
   fwrite(da->base, sizeof(int), da->length, fp);
   fwrite(da->check, sizeof(int), da->length, fp);
   fwrite(da->values, sizeof(int), da->length, fp);
   
   fclose(fp);
   
   return DARY_STATUS_SUCCESS;
}


void
dary_free(dary *da)
{
   if(da->can_free) {
      free(da->base);
      free(da->check);
      free(da->values);
   } else {
      munmap(da->base, da->length * sizeof(int) * 3);
   }
   free(da);
}

dary_status
dary_step(dary *da, int *pos, uchar k)
{
   int n = *pos;
   int m;
   int *base = da->base;
   int *check = da->check;
   int *values = da->values;
  
   m = base[n] + k;

   if(m < 0) {
      *pos = -1;
      return DARY_STATUS_FAIL;
   }

   if(check[m] == n) {
      *pos = m;
      return DARY_STATUS_SUCCESS;
   } else {
      *pos = -1;
      return DARY_STATUS_FAIL;
   }
}

dary_status
dary_exact_search_pos(dary *da, const uchar *key, int *pos)
{
   int *base = da->base;
   int *check = da->check;
  
   int i = 0;
   int m;
   int n = 0;

   while(1) {
      
      m =  base[n] + key[i];
      
      if (check[m] != n) {
         //      printf("search failed for key %s (m=%d check[m]=%d n=%d)\n", key, m, check[m], n);
         return DARY_STATUS_FAIL;
      }

      if(key[i+1] == '\0') {
         //      printf("found for key %s in %d\n", key, m);
         *pos= m;
         return DARY_STATUS_SUCCESS;
      }

      n = m;
      i++;
   }

   return DARY_STATUS_FAIL;
}

dary_status
dary_inexact_search_pos(dary *da, const uchar *key, int *pos)
{
   int *base = da->base;
   int *check = da->check;
  
   int i = 0;
   int m;
   int n = 0;

  
   while(1) {

      m =  base[n] + key[i];
      /*    if(m >= da->length) {
            return DARY_STATUS_FAIL;
            }*/
      if(check[m] != n) {
         *pos= m;
         return DARY_STATUS_SUCCESS;
      }

      if(key[i+1] == '\0') {
         //      printf("found for key %s in %d\n", key, m);
         *pos= m;
         return DARY_STATUS_SUCCESS;
      }

      n = m;
      i++;
   }

   return DARY_STATUS_FAIL;
}

static int
length(int *array)
{
   int i = 0;
   while (array[i] != -1) {
      i++;
   }
   return i;
}

int
dary_prefix_search(dary *da, const char *prefix, int **r)
{
   int index;
   int *result = (int*)malloc(sizeof(int));
   int retcode = dary_exact_search_pos(da, prefix, &index);
   int *unchecked;

   if (retcode != DARY_STATUS_SUCCESS) {
      result[0] = -1;
      *r = result;
      return DARY_STATUS_FAIL;
   }

   unchecked = (int*)malloc(sizeof(int)*2);
   unchecked[0] = index;
   unchecked[1] = -1;
   result[0] = -1; 

   while (unchecked[0] != -1) {
      if (dary_get_value(da, unchecked[0])) {
         int result_len = length(result);
         result = realloc(result, (result_len + 2) * sizeof(int));
         result[result_len] = unchecked[0];
         result[result_len+1] = -1;
      }
      int children[256+1];
      get_children(da, unchecked[0], children);
      
      if (children[0] != -1) {
         int newlen = length(unchecked) + length(children) + 1;
         unchecked = realloc(unchecked, newlen * sizeof(int));
         memmove(unchecked + length(children), unchecked + 1, length(unchecked) * sizeof(int));
         memmove(unchecked, children, length(children) * sizeof(int));
      } else {
         int len = length(unchecked);
         memmove(unchecked, unchecked + 1, len * sizeof(int));
      }
   }
   free(unchecked);
   
   *r = result;
   return DARY_STATUS_SUCCESS;
}

dary_status
dary_common_prefix_search(dary *da, const uchar *key, int **r)
{
   int *base = da->base;
   int *check = da->check;
   
   int i = 0;
   int m;
   int n = 0;
   int *result = (int*)malloc(sizeof(int));   

   result[0] = -1;

   while (1) {
      m =  base[n] + key[i];
      //      dprint("m = %d = base[%d] + key[%d] = %d + %d", m, n, i, base[n], key[i]);
      assert(m < da->length);

      if (check[m] != n) {

         if (result[0] != -1) {
            *r = result;
            return DARY_STATUS_SUCCESS;
         } {
            return DARY_STATUS_FAIL;
         }
      }

      if (dary_get_value(da, m)) {
         int len = length(result);
         result = realloc(result, (len + 2) * sizeof(int));
         result[len] = m;
         result[len+1] = -1;
      }
      
      if (key[i+1] == '\0') {
         if (result[0] != -1) {
            *r = result;
            return DARY_STATUS_SUCCESS;
         } {
            return DARY_STATUS_FAIL;
         }
      }
      n = m;
      i++;
   }
   return DARY_STATUS_FAIL;
}


dary_status
dary_exact_search(dary *da, const uchar *key, int *value)
{
   int *base = da->base;
   int *check = da->check;
   int *values = da->values;
  
   int i = 0;
   int m;
   int n = 0;

   *value = NULL;
  
   while (1) {

      m =  base[n] + key[i];
   
      dprint("m = %d (base[%d](%d) + key[%d](%d))", m, n, base[n], i, key[i]);

      assert(m < da->length);

      if (check[m] != n) {
         dprint("  search failed for key %s (m=%d check[m]=%d n=%d)\n", key, m, check[m], n);
         return DARY_STATUS_FAIL;
      }

      if(key[i+1] == '\0') {
         //      printf("found for key %s in %d\n", key, m);
         *value = values[m];
         return DARY_STATUS_SUCCESS;
      }

      n = m;
      i++;
   }

   return DARY_STATUS_FAIL;
}

static int
get_children_num(dary *da, int index)
{
   int i, j = 0;
   int *base = da->base;
   int *check = da->check;
   int search_start = MAX(base[index], 1);
   int search_limit = MIN(base[index]+256, da->length); /* 255で十分かも？ */
  
   dprint("get_children_num search_start:%d", search_start);

   for (i=search_start; i < search_limit; i++) {
      if (check[i] == index) { /* This condition means index == index of parent */
         j++;
      }
   }
   return j;
}


/* dary_get_childrenとの違いはメモリを確保しないこと。caller側で確保してlocに渡す必要がある */
static int
get_children(dary *da, int index, int *loc)
{
   int i, j = 0;
   int *base = da->base;
   int *check = da->check;
   int search_start = MAX(base[index], 1);
   int search_limit = MIN(base[index]+256, da->length);
  
   dprint("get_children search_start:%d", search_start);

   for (i=search_start; i < search_limit; i++) {
      if (check[i] == index) { /* This condition means index == index of parent */
         loc[j] = i;
         j++;
      }
   }

   loc[j] = -1;
   return j;
}

dary_status
dary_get_children(dary *da, int index, int **loc)
{
   int *children;

   children = malloc(sizeof(int) * (256 + 1));
   if(children == NULL) {
      return DARY_STATUS_NO_MEMORY;
   }

   get_children(da, index, children);

   *loc = children;

   return DARY_STATUS_SUCCESS;
}


static int
find_free_node(dary *da, const uchar minimum)
{
   int i;

   dprint("find_free_node:%d", da->free_head);

   if (da->free_head < 0) {
      return -1;
   }

   for (i = da->free_head; da->check[i] != -i; i = - da->check[i]) {
      dprint("::%d", i);
      if (i > minimum && is_free_node(da, i)) {
         //printf("find_free_node: unused node for %d found in %d\n", key, i);
         return i;
      }
   }
   if (da->check[i] == -i) {
      return i;
   }

   return -1;
}

static void
insert_node_to(dary *da, int m, int n, int k)
{
   dprint("insert_node_to %d k = %d, from %d (base %d, check %d)", m, k, n, da->base[n], da->check[n]);

   remove_from_freelist(da, m);
  
   da->check[m] = n;
}

static int
assign_base(dary *da, int n, uchar k)
{
   int free_node = find_free_node(da, k);
  
   if (free_node >= k) {
      da->base[n] = free_node - k;
      dprint("found freenode in %d", free_node);
      return free_node;
   } else {
      dprint("freenode not found");
      da->base[n] = da->length - k;
      return da->length;
   }
}


static int
is_movable(dary *da, int index, const int *offsets)
{
   int i;
   dprint("index + uchar_max = %d, length = %d", index + UCHAR_MAX, da->length);
   if (index + UCHAR_MAX >= da->length) {
      return 0;
   }
   for (i =0; offsets[i] != -1; i++) {
      dprint("checking %d", index + offsets[i]);
      if (!is_free_node(da, index + offsets[i])) {
         return 0;
      }
   }
   return 1;
}

/* Convert index array to offset array to first node. */
/* FIXME: not appropriate function name. */
static void
calc_offsets(const int *nodes, int *offsets)
{
   int f = nodes[0]; /* f means First value of nodes. */
   dprint("calc_offsets, f = %d", f);
   while (*nodes != -1) {
      *offsets = *nodes - f;
      dprint("offsets %d for %d", *offsets, *nodes);
      offsets++; nodes++;
   }
   *offsets = -1;
}

static dary_status
find_movable_location(dary *da, const int *conflicts, int *first_loc)
{
   int i = da->free_head;
   int offsets[UCHAR_MAX+1];

   calc_offsets(conflicts, offsets);

   //  dary_dump(da);
   //  validate_array(da);

   dprint("free_head %d", da->free_head);

   if (da->free_head < 0) {
      *first_loc = -1;
      return DARY_STATUS_FAIL;
   }

   while (i != - da->check[i]) {

      assert(i > UCHAR_MAX);
      
      if (is_movable(da, i, offsets)) {
         dprint("movable to %d", i);
         *first_loc = i;
         return DARY_STATUS_SUCCESS;
      }
      i = - da->check[i];
   }

   if (is_movable(da, i, offsets)) {
      dprint("movable to %d", i);
      *first_loc = i;
      return DARY_STATUS_SUCCESS;
   } else {   
      *first_loc = -1;
      return DARY_STATUS_FAIL;
   }
}

static void
return_to_freelist(dary *da, int index)
{
   int prev_free_node;

   dprint("return to freelist %d", index);

   assert(index > UCHAR_MAX);

   if (da->free_head < 0) {
      da->free_head = index;
      da->check[index] = -index;
      return;
   }

   prev_free_node = find_prev_free_node(da, index);
  
   if (prev_free_node > 0) {
      if (is_last_free_node(da, prev_free_node)) {
         da->check[index] = - index;
      } else {
         da->check[index] = da->check[prev_free_node];
      }
      da->check[prev_free_node] = - index;
   } else {
      dprint("index = %d, da->free_head = %d", index, da->free_head);
      da->check[index] = - da->free_head;
      da->free_head = index;
   }

   da->base[index] = 0;
   da->values[index] = 0; /* これも本当はなくしたい */
}


static void
move_node(dary *da, int from, int to)
{
   int i;
   /* FIXME: Simply da->base[from] is enough is implemented correctly. */
   int search_start = MAX(da->base[from] , 1);
   int search_limit = MIN(da->base[from] + 256, da->length);

   dprint("move node from %d to %d", from, to);

   remove_from_freelist(da, to);

   if (da->base[from] == 0) {
      da->base[to] = 0;
   } else {
      da->base[to] = da->base[from];
      dprint("da->base[%d] = %d\n", to, da->base[from]);
   }

   da->check[to] = da->check[from];

   for (i = search_start; i < search_limit; i++) {
      if (da->check[i] == from) {
         da->check[i] = to;
      }
   }

   da->values[to] = da->values[from];
   da->values[from] = 0;  /* FIXME: This line should be removed */

   return_to_freelist(da, from);
}

/* This function does no error check. You must check and ensure this function
   must not fail before calling. */
static void
move_conflicts_real(dary *da, const int *conflicts, int move_to)
{
   int i = 0;
   int offsets[257];
   int parent_index = get_parent_index(da, conflicts[0]);

   calc_offsets(conflicts, offsets);

   da->base[parent_index] = move_to - (conflicts[0] - da->base[parent_index]);

   for (i = 0; conflicts[i] != -1; i++) {
      move_node(da, conflicts[i], move_to + offsets[i]);
   }
   //  dary_dump(da);
}

static dary_status
move_conflicts(dary *da, int want_to_be_free_index, int *n)
{
   int conflicts[256+1], move_to; /* MAX conflicts 256, +1 is for termination */
   int retcode;
   int parent = get_parent_index(da, want_to_be_free_index);
   int i = 0;

   dprint("want_to_be_free is %d and its parent is %d",  want_to_be_free_index, parent);

   get_children(da, parent, conflicts);
   /* coflicts are want_to_be_free and its siblings.
      (For non-English speaker like me: Siblings means brothers and sisters.) */
  
   retcode = find_movable_location(da, conflicts, &move_to);

   if (retcode != DARY_STATUS_SUCCESS) {
      dprint("move_conflicts:Could not found movable location. Retrying...\n");
      retcode = extend_array(da, da->length + 256);
      if (retcode != DARY_STATUS_SUCCESS) {
         return retcode;
      }
      /* This find_movable_loc must be success, but ensure retcode to make sure. */
      retcode = find_movable_location(da, conflicts, &move_to);
    
      if (retcode != DARY_STATUS_SUCCESS) {
         return retcode;
      }
   }

   while (conflicts[i] != -1) {
    
      if (conflicts[i] == *n) {
         dprint("parent %d moved to %d\n", parent, parent + (move_to - conflicts[0]));
         *n = *n + (move_to - conflicts[0]);
         break;
      }

      dprint("confl[%d], %d  ", i, conflicts[i]);
      i++;
   }
   move_conflicts_real(da, conflicts, move_to);

   return DARY_STATUS_SUCCESS;
}

static dary_status
move_conflicts2(dary *da, int n, uchar k)
{
   int conflicts[256+1], move_to; /* MAX conflicts 256, +1 is for termination */
   int retcode;
   int i = 0;

   get_children(da, n, conflicts);

   dprint("da->base[%d] =%d, k = %d", n, da->base[n], k);
  
   // ここはなんとか関数化できないか？
   if (conflicts[0] < da->base[n] + k) {
      while (conflicts[i] != -1) {
         i++;
      }
      dprint("last node is k");
      conflicts[i] = da->base[n] + k;
      conflicts[i+1] = -1;
   } else {
      int len = length(conflicts);
      memmove(&conflicts[1], conflicts, (len + 1) * sizeof(int));
      conflicts[0] = da->base[n] + k;
      dprint("first node is k");
   }
   i = 0;
   while (conflicts[i] != -1) {
      dprint("conflicts[%d] = %d", i, conflicts[i]);
      i++;
   }

   retcode = find_movable_location(da, conflicts, &move_to);

   if (retcode != DARY_STATUS_SUCCESS) {
      dprint("move_conflicts:Could not found movable location. Retrying...\n");
      retcode = extend_array(da, da->length + 256);
      if (retcode != DARY_STATUS_SUCCESS) {
         return retcode;
      }
      /* This find_movable_loc must be success, but ensure return value to make sure. */
      retcode = find_movable_location(da, conflicts, &move_to);
    
      if (retcode != DARY_STATUS_SUCCESS) {
         return retcode;
      }
   }

   i = 0;
   int offsets[257];

   calc_offsets(conflicts, offsets);

   for (i = 0; conflicts[i] != -1; i++) {
      if (conflicts[i] != da->base[n] + k) {
         move_node(da, conflicts[i], move_to + offsets[i]);
      }
   }
   da->base[n] = move_to - (conflicts[0] - da->base[n]);
   dprint("da->base[%d] = da->base[%d] (%d) + move_to %d - %d = %d",
          n, n, da->base[n], move_to, conflicts[0], 
          da->base[n] + (move_to - conflicts[0]));

   return DARY_STATUS_SUCCESS;
}

static int
should_move_m(dary *da, int m, int n)
{
   if (m <= UCHAR_MAX) {
      return 0;
   }

   if (get_children_num(da, get_parent_index(da, m)) >=
       get_children_num(da, n)) {
      return 0;
   } else {
      return 1;
   }
}

dary_status
dary_insert(dary *da, const uchar *key, int value)
{
   int i, n, m;
   n = key[0];
   dprint("key:%s", key);
   dprint("free_head:%d", da->free_head);

   // validate_array(da); 
   for (i = 1; key[i] != '\0';i++) {

      if (is_leaf_node(da, n)) {
         m = assign_base(da, n, key[i]);
      } else {    
         m = da->base[n] + key[i];
      }
      dprint("m = %d (base[%d](%d) + key[%d](%d))", m, n, da->base[n], i, key[i]);
    
      if (m >= da->length) {
         int retcode = extend_array(da, m + 256);
        
         if (retcode != DARY_STATUS_SUCCESS)
            return retcode;
      }
      
      if (is_free_node(da, m)) {
         dprint("%d is free node, insert", m);
         dprint("key[%d] = %d", i, key[i]);
         insert_node_to(da, m, n, key[i]);
      } else if (is_no_conflict(da, m, n, key[i])) {
         dprint("no conflict for key[%d]", i);
      } else {
         /* FIXME: Here should be added a check for which is easy to move. */

         if (should_move_m(da, m, n)) {
            dprint("move m");
            int retcode = move_conflicts(da, m, &n);
            
            if (retcode != DARY_STATUS_SUCCESS) {
               dprint("failed to move conflicted node %d\n",m);
               return retcode;
            }
            insert_node_to(da, m, n, key[i]);
         } else {
            dprint("move n, n=%d, base[n] = %d", n, da->base[n]);
            int retcode = move_conflicts2(da, n, key[i]);
            dprint("current n=%d, base[n] = %d", n, da->base[n]);
            if (retcode != DARY_STATUS_SUCCESS) {
               dprint("failed to move conflicted node %d\n",m);
               return retcode;
            }
            m = da->base[n] + key[i];
            insert_node_to(da, m, n, key[i]);

         }
      }
      //    fprintf(stderr, "\n");
      n = m;
   }
  
   da->values[n] = value;
   //  validate_array(da);
   //  dary_dump(da);
  
   return DARY_STATUS_SUCCESS;
}

dary_status
dary_insert_len(dary *da, const uchar *key, int key_len, int value)
{
   return DARY_STATUS_FAIL;
}


dary_status
dary_delete(dary *da, const uchar *key)
{
   return DARY_STATUS_FAIL;
}

void
dary_dump(const dary *da)
{
   dary_dump_range(da, 1, da->length);
}

void
dary_dump_range(const dary *da, int start, int end)
{
   int i;
   start = MAX(start, 1);
   end = da->length;
   printf("dump from %d to %d: \n", start, end);
  
   for(i = start; i<end; i++) {

      if (da->check[i] < 0) {
         printf(" %3d:%4d:%4d|", i, da->base[i], da->check[i]);
      } else {
         printf(" %3d:%4d:%4d|", i, da->base[i], da->check[i]);
      }

      if (i % 10 == 0) {
         printf("\n");
      }
    
   }
   printf("\n");
}


void
dary_print_stat(dary *da)
{
   int i;
   int used = 0;

   for(i = 0; i<da->length; i++) {
      if(da->check[i] > 0) {
         used++;
      }
   }

   printf("dary_print_stat\n");
   printf("  length of double array: %d\n", da->length);
   printf("  size of double array  : %d KB\n", da->length * 12 / 1000);
   /* 3 times for base and check and value, 4 time because one node size is 4byte. */
   printf("  used nodes            : %d  (%3.2lf percent) \n", used, (double)used/(da->length) * 100);
}

struct _dary_cb{
   FILE *fp;
   int pos;
};

dary_cb *
dary_countbuffer_new(const char *filename)
{
   FILE *fp;
   dary_cb *cb;
   
   fp = fopen(filename, "w");
   if(!fp)
      return NULL;
   
   cb = malloc(sizeof(dary_cb));

   if(!cb)
      return NULL;

   cb->fp = fp;
   cb->pos = 0;

   return cb;
}

int
dary_countbuffer_append(dary_cb *cb, const char *str)
{
   int pos = cb->pos;
   int str_len = strlen(str) + 1;
   char nullchar = '\0';
   fputs(str, cb->fp);
   fwrite(&nullchar, 1, 1, cb->fp);
   cb->pos = pos + str_len;

   return pos;
}

void
dary_countbuffer_free(dary_cb *cb)
{
   fclose(cb->fp);
   free(cb);
}

/*
 * Local Variables: 
 * coding: utf-8
 * c-basic-offset: 3
 * tab-width: 8
 * End:
 */
