/*
  Dary - Double Array library
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
#include <dary.h>

#include <stdio.h>
#include <string.h>

void
search_test(dary *da, const uchar *key, int val_expected)
{
  int val;
  dary_exact_search(da, key, &val);
  if(val != val_expected)
    g_printerr("search failed. Expected %d but was %d\n",val_expected, val);
}

int
main(int argc, char *argv[]) {

  dary *da = dary_new();
  char buffer[8192];
  int i;
  int length = 0;

  GTimer *timer = g_timer_new();

  for(i=0;i<1e6;i++) {
    sprintf(buffer, "%d", i);
    dary_insert(da, (uchar*)buffer, 1);
    length += strlen(buffer);
  }

  g_print("%lf sec\n",g_timer_elapsed(timer, NULL));
  //  dary_dump(da);
  g_print("len %d\n",length);
  dary_print_stat(da);
  return 0;
}
