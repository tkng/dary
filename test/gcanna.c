/*

  Copyright (c) 2006 TOKUNAGA Hiroyuki <tkng@xem.jp>

  All rights reserved.

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
#include <unistd.h>

#define BUFFER_SIZE 8192

static guchar *
forward_until_next_space(guchar *line)
{
  guchar *ptr = line;
  
  while(ptr != NULL) {
    if(*(ptr+1) == ' ' && *ptr != '\\') {
      break;
    }
    ptr++;
  }
  return ptr;
}

guchar *
get_word(guchar *line)
{
  guchar *a = forward_until_next_space(line);
  guchar *b;
  a++;
  a = forward_until_next_space(a);
  a+=2;
  b = forward_until_next_space(a);
  b++;
  int len = b - a;
  return (guchar*)strndup(a, len);
}

guchar *
get_pron(guchar *line)
{
  guchar *ptr = forward_until_next_space(line);
  int len;
  ptr++;
  len = ptr - line;
  return (guchar*)strndup(line, len);
}
