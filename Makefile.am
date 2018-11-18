ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CFLAGS = $(WARN_CFLAGS)

sbin_PROGRAMS = gcrond

gcrond_SOURCES = gcrond.c gcrond.h rdcron.c crontab.c cronscan.c rdline.c

crontabdir = @GCRONDIR@
crontab_DATA = crontab/0-example

EXTRA_DIST = crontab/0-example LICENSE README.md
