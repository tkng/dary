AM_CFLAGS= -W -Wall

lib_LTLIBRARIES = libdary.la

include_HEADERS = dary.h

libdary_la_SOURCES = dary.c dary.h debug.c debug.h
libdary_la_CFLAGS = -I$(top_srcdir)/src
libdary_la_LDFLAGS = -export-symbols-regex dary_.\*

noinst_PROGRAMS = darybuild darysearch

darybuild_SOURCES = build.c
darybuild_LDADD = -ldary libdary.la

darysearch_SOURCES = search.c
darysearch_LDADD = -ldary libdary.la
