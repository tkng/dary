noinst_PROGRAMS = 

LDADD = -L$(top_builddir)/src

if GLIB
  noinst_PROGRAMS += hash-gcanna da-gcanna test1

  hash_gcanna_SOURCES = hash-gcanna.c gcanna.c
  hash_gcanna_CFLAGS = @GLIB_CFLAGS@ -g -Wall
  hash_gcanna_LDADD = @GLIB_LIBS@

  da_gcanna_SOURCES = da-gcanna.c gcanna.c
  da_gcanna_CFLAGS = -I$(top_srcdir)/src @GLIB_CFLAGS@
  da_gcanna_LDADD = @GLIB_LIBS@ -ldary -L$(top_builddir)/src

  test1_SOURCES = test1.c
  test1_CFLAGS = -I$(top_srcdir)/src @GLIB_CFLAGS@
  test1_LDADD = @GLIB_LIBS@ -ldary  -L$(top_builddir)/src

endif
