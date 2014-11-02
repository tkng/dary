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
#include <stdlib.h>
#include <string.h>

static void
print_usage(void)
{
   printf("Usage: darysearch index\n");
}

int
main(int argc, char *argv[]) {

   FILE *index;
   dary *da;
   char buf[8192];

   if (argc != 2) {
      print_usage();
      exit(EXIT_FAILURE);
   }

   da = dary_new_from_file(argv[1]);

   if (!da) {
      exit(EXIT_FAILURE);
   }

   while (fgets(buf, sizeof(buf), stdin) != NULL) {
      int len = strlen(buf);
      buf[len - 1] = '\0';
      int *results;
      int *p;

      dary_status retcode = dary_common_prefix_search(da, buf, &results);
      
      if (retcode == DARY_STATUS_SUCCESS) {
         p = results;
         while (*p != -1) {
            char *key = dary_get_key(da, *p);
            printf("%s\n", key);
            p++;
         }
      } else {
         printf("Not found!\n");
      }
      free(results);
   }

   dary_free(da);

   return 0;
}


/*
 * Local Variables: 
 * coding: utf-8
 * c-basic-offset: 3
 * tab-width: 8
 * End:
 */
