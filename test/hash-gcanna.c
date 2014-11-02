/*
  Test code with hash, for comparing with double array.

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

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 8192

double
measure_all_word_search_time(GHashTable *hash, const char *filename)
{
  GTimer *timer = g_timer_new();

  guchar *searched_value;
  guchar *word;
  FILE *fp;
  guchar buf[BUFFER_SIZE];
  fp = fopen(filename, "r");
  g_timer_start(timer);

  while (fgets(buf, BUFFER_SIZE, fp) != NULL) {
    word = get_pron(buf);
    g_hash_table_lookup(hash, word);
    
    free(word);
  }

  fclose(fp);
  return g_timer_elapsed(timer, NULL);
}

int
main(int argc, char *argv[]) {
  
  GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
  FILE *fp;
  guchar *word;
  guchar buf[BUFFER_SIZE];
  guchar *searched_value;
  int i = 0;
  double elapsed;
  
  GTimer *timer = g_timer_new();
  g_timer_start(timer);

  fp = fopen("gcanna.ctd.utf-8", "r");
  
    while (fgets(buf, BUFFER_SIZE, fp) != NULL) {
      i++;
      word = get_pron(buf);
      //      printf("%s, %d\n",word, i);

      g_hash_table_insert(hash, word, NULL);

      //      g_hash_table_lookup(hash, word);

      /*
      if(i%1000 == 0) {
	elapsed = g_timer_elapsed(timer, NULL);
	printf("consumed %lf seconds until %d words\n", elapsed, i);
      }
      */
      //      free(word);

    }
  fclose(fp);
  elapsed = g_timer_elapsed(timer, NULL);
  printf("consumed %lf seconds to insert %d words\n", elapsed, i);

  elapsed = measure_all_word_search_time(hash, "gcanna.ctd.utf-8");


  printf("consumed %lf seconds to search %d words\n", elapsed, i);
  //  int hashsize = g_hash_table_size2(hash);
  //  printf("hash size:%d\n",hashsize);

  return 0;
}
