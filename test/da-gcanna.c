/*
  Test code with gcanna. (You need to get gcanna.ctd from somewhere.)

  Copyright (C) 2006 TOKUNAGA Hiroyuki

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

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 8192

void
write_to_file(dary *da)
{
  dary_write_to_file(da, "gcanna.v");
}

dary *
load_from_file(void)
{
  double elapsed;
  GTimer *timer = g_timer_new();

  dary *da = dary_new_from_file("gcanna.v");

  if (da) {
    elapsed = g_timer_elapsed(timer, NULL);
    printf("consumed %lf seconds to load from files\n", elapsed);
    return da;
  } else {
    printf("failed to load gcanna.v\n");
    return NULL;
  }
}

double
measure_all_word_search_time(dary *da, const char *filename)
{
  GTimer *timer = g_timer_new();
  int result;
  uchar *pron;
  uchar *word;
  FILE *fp;
  uchar buf[BUFFER_SIZE];
  uchar *content;
  gsize content_length;
  int i=0;
  g_file_get_contents("gcanna-content", &content, &content_length, NULL);
  fp = fopen(filename, "r");
  g_timer_start(timer);

  if(!content) {
    g_print("Open failed for gcanna-content\n");
  }

  while (fgets(buf, BUFFER_SIZE, fp) != NULL) {
    i++;
    pron = get_pron(buf);
      
    if(dary_exact_search(da, pron, &result) == DARY_STATUS_FAIL) {
      printf("Error:failed to search %s\n",pron);
      exit(-1);
    }

    if (result != i) {
      printf("Error: Expected %d but is %d\n", i , result);
    }
    word = get_word(buf);

    if(content) {
#if 0
      if(strcmp(word, content + result) != 0) {
	printf("Error:failed to search %s, content doesn't same.\n",pron);
      }
#endif
    }

    free(word);
  }
  fclose(fp);
  return g_timer_elapsed(timer, NULL);
}


int
main(int argc, char *argv[]) {
  
  dary *da;
  FILE *fp;
  uchar *pron;
  uchar *word;
  uchar buf[BUFFER_SIZE];
  uchar *searched_value;
  int i = 0;
  int wordslength = 0;
  double elapsed;
  GTimer *timer = g_timer_new();

  if (da = load_from_file()) {
    elapsed = measure_all_word_search_time(da, "gcanna.ctd.utf-8");
    printf("consumed %lf seconds to search all words\n", elapsed);
  } else {

    dary_cb *cb;
    int pos;
    da = dary_new();
    cb = dary_countbuffer_new("gcanna.s");

    g_timer_start(timer);
    
    fp = fopen("gcanna.ctd.utf-8", "r");
    
    while (fgets(buf, BUFFER_SIZE, fp) != NULL) {
      int result;
      i++;
      pron = get_pron(buf);
      word = get_word(buf);
      
      pos = dary_countbuffer_append(cb, word);
      int retcode = dary_insert(da, (uchar*)pron, i);

      if (retcode != DARY_STATUS_SUCCESS) {
        printf("failed to insert %s. Error code:%d\n", word, retcode);
      }
      
      if (dary_exact_search(da, pron, &result) == DARY_STATUS_FAIL) {
        printf("failed to insert %s (%s), it cannot retrieved after insertion.\n", pron, word);
        abort();
      }
      
      wordslength += strlen(word)+1;
      
      g_free(pron);
      g_free(word);
    }
    fclose(fp);
    dary_countbuffer_free(cb);
    
    elapsed = g_timer_elapsed(timer, NULL);
    printf("consumed %lf seconds to insert %d words\n", elapsed, i);
    
    dary_print_stat(da);
    
    elapsed = measure_all_word_search_time(da, "gcanna.ctd.utf-8");
    printf("consumed %lf seconds to search %d words\n", elapsed, i);
    printf("length of words %d\n", wordslength);

    g_timer_start(timer);
    write_to_file(da);
    elapsed = g_timer_elapsed(timer, NULL);
    printf("consumed %lf seconds to write %d words into the files.\n", elapsed, i);
  }
  return 0;
}

