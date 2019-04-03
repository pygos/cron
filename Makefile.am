ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CFLAGS = $(WARN_CFLAGS)

sbin_PROGRAMS = gcrond

gcrond_SOURCES = gcrond.c gcrond.h rdcron.c crontab.c cronscan.c
gcrond_CPPFLAGS = $(AM_CPPFLAGS)
gcrond_CFLAGS = $(AM_CFLAGS)
gcrond_LDADD =

if HAVE_LIBBSD
gcrond_CPPFLAGS += -DHAVE_LIBBSD
gcrond_CFLAGS += $(LIBBSD_CFLAGS)
gcrond_LDADD += $(LIBBSD_LIBS)
endif

crontabdir = @GCRONDIR@
crontab_DATA = crontab/0-example

EXTRA_DIST = crontab/0-example LICENSE README.md
