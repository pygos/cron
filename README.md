# About

This package contains a small cron implementation called `gcrond`.

It was written due to a perceived lack of a proper, simple cron
implementation. All other cron implementation I came across were either decade
old, abandoned pieces of horror ("Cool, I didn't even know that C syntax
allows this!") or hopelessly integrated into other, much larger projects (e.g.
absorbed by SystemD or in the case of OpenBSD cron, married to special OpenBSD
syscalls).

It was a fun little exercise and it seems to work so far. No idea about
standards compliance tough, the implementation was mostly written against
the Wikipedia article about Cron.

## License

The source code in this package is provided under the OpenBSD flavored ISC
license. So you can practically do as you wish, as long as you retain the
original copyright notice. The software is provided "as is" (as usual) with
no warranty whatsoever (e.g. it might actually do what it was designed for,
but it could just as well set your carpet on fire).

The sub directory `m4` contains third party macro files used by the build
system which may be subject to their own, respective licenses.


## Portability

The program in this package has been written for and tested on a GNU/Linux
system, so there may be some GNU-isms in there in addition to Linux specific
code. Depending on your target platform, some minor porting effort may be
required.


# Building and installing

This package uses autotools. If you downloaded a distribution tar ball, simply
run the `configure` script and then `make` after the Makefile has been
generated. A list of possible `configure` options can be viewed by running
`configure --help`.

If you really wish to do so, run `make install` to install the program on your
system.

When working with the git tree, run the `autogen.sh` script to generate the
configure script and friends.


# Crontab File Format

The cron daemon reads its configuration from all files it can find
in `/etc/crontab.d/` (exact path can be configured).

The files are read line by line. Empty lines or lines starting with '#' are
skipped.

Each non-empty line consists of the typical cron fields:

1. The `minute` field. Legal values are from 0 to 59.
2. The `hour` field. Legal values are from 0 to 23.
3. The `day of month` field. Legal values are from 1 to 31 (or fewer, depending
   on the month.
4. The `month` field. Legal values are from 1 to 12 (January to December)
   or the mnemonics `JAN`, `FEB`, `MAR`, `APR`, ...
5. The `day of week` field. Legal values are from 0 to 6 (Sunday to Saturday)
   or the mnemonics `SUN`, `MON`, `TUE`, `WED`, ...
6. The command to execute.


The fields are separated by spaces. For the time matching fields, multiple
comma separated values can be specified (e.g. `MON,WED,FRI` for a job that
should run on Mondays, Wednesdays and Fridays).

The wild-card character `*` matches any legal value. An stepping can be
specified by appending `/` and then a stepping (e.g. for the minute field,
`*/5` would let a job run every five minutes).

A range of values can also be specified as `<lower>-<upper>`, for instance
`MON-FRI` would match every day from Monday to Friday (equivalent to `1-5`).

Intervals and specific values can be combined, for instance a day of month
field `*/7,13,25` would trigger once a week, starting from the first of the
month (1,7,14,21,28), but additionally include the 13th and the 25th. The
same could be expressed as `1-31/7,13,25`.


Instead of specifying a terse cron matching expression, the first five fields
can be replaced with one of the following mnemonics:

- `@yearly` or `@anually` is equivalent to `0 0 1 1 *`, i.e. 1st of January
  at midnight
- `@monthly` is equivalent to `0 0 1 * *`, i.e. 1st of every month at midnight
- `@weekly` is equivalent to `0 0 * * 0`, i.e. every Sunday at midnight
- `@daily` is equivalent to `0 0 * * *`, i.e. every day at midnight
- `@hourly` is equivalent to `0 * * * *`, i.e. every first minute of the hour

Lastly, the command field is not broken down but passed to `/bin/sh -c`
*as is*.


# Security Considerations

The cron daemon currently has no means of specifying a user to run the jobs as,
so if cron runs as root, the jobs it starts do as well. Since by default it
reads its configuration from `/etc` which by default is only writable by root,
this shouldn't be too much of a problem when using cron for typical system
administration tasks.

If a job should run as another user, tools such as `su`, `runuser`, `setpriv`
et cetera need to be used.

# Possible Future Directions

The following things would be nice to have:

- decent logging for cron and the output of the jobs.
- cron jobs per user, e.g. scan `~/.crontab.d` or similar and run the collected
  jobs as the respective user.
- timezone handling
- some usable strategy for handling time jumps, e.g. caused by a job that
  syncs time with an NTP server on a system without RTC.
