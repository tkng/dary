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

#include <dary.h>
#include <stdio.h>

void
search_test(dary *da, const uchar *key, int val_expected)
{
  int val;
  dary_exact_search(da, key, &val);

  if(val == val_expected) {
    printf("search successed for key %s.\n",key);
  } else {
    printf("search failed for key %s. Expected %d but was %d\n",key, val_expected, val);
  }
}

int
main(int argc, char *argv[]) {
  int val;
  int *result;
  dary *da = dary_new();

  dary_insert(da, (uchar*)"a",  9);
  search_test(da, "a", 9);
  dary_insert(da, (uchar*)"aa",  15);
  search_test(da, "a", 9);
  search_test(da, "aa", 15);
  dary_insert(da, (uchar*)"aaa", 11);
  dary_insert(da, (uchar*)"ab",  12);
  // dary_insert(da, (uchar*)"abc", (uchar*)"a");
  dary_insert(da, (uchar*)"ba",  13);
  //  dary_insert(da, (uchar*)"foo", (uchar*)"");


  printf("dary_get_key:%s\n", dary_get_key(da, 98));


  search_test(da, "aaa", 11);
  search_test(da, "ab", 12);
  search_test(da, "ba", 13);



  printf("prefix search result for key \"a\"\n");
  if (dary_prefix_search(da, "a", &result) == DARY_STATUS_SUCCESS) {
    int i = 0;
    while (result[i] != -1) {
      printf(" result[%d] = %d (key:%s)\n", i, result[i], dary_get_key(da, result[i]));
      i++;
    }
    free(result);
  }

  printf("common prefix search result for key \"aaa\"\n");
  if (dary_common_prefix_search(da, "aaa", &result) == DARY_STATUS_SUCCESS) {
    int i = 0;
    while (result[i] != -1) {
      printf(" result[%d] = %d (key:%s)\n", i, result[i], dary_get_key(da, result[i]));
      i++;
    }
    free(result);
  }

  dary_exact_search(da, (uchar*)"aab", &val);
  //  printf("val:%s\n",val);
  dary_exact_search(da, (uchar*)"ab", &val);
  dary_exact_search(da, (uchar*)"ba", &val);
  dary_exact_search(da, (uchar*)"aac", &val);
  //  printf("val:%s\n",val);
  dary_dump(da);
  dary_print_stat(da);
  return 0;
}
