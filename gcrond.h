/* SPDX-License-Identifier: ISC */
#ifndef GCROND_H
#define GCROND_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <dirent.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include "config.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef struct crontab_t {
	struct crontab_t *next;
	char *exec;

	uint64_t minute;
	uint32_t hour;
	uint32_t dayofmonth;
	uint16_t month;
	uint8_t dayofweek;
} crontab_t;

typedef struct {
	const char *filename;	/* input file name */
	size_t lineno;		/* current line number */
	FILE *fp;
	char *line;
} rdline_t;

int rdline_init(rdline_t *t, int dirfd, const char *filename);

void rdline_complain(rdline_t *t, const char *msg, ...);

void rdline_cleanup(rdline_t *t);

int rdline(rdline_t *t);

crontab_t *rdcron(int dirfd, const char *filename);

void delcron(crontab_t *cron);

int cronscan(const char *directory, crontab_t **list);

void cron_tm_to_mask(crontab_t *out, struct tm *t);

bool cron_should_run(const crontab_t *t, const crontab_t *mask);

int runjob(crontab_t *tab);

#endif /* GCROND_H */
