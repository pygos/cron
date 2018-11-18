/* SPDX-License-Identifier: ISC */
#include "gcrond.h"

void cron_tm_to_mask(crontab_t *out, struct tm *t)
{
	memset(out, 0, sizeof(*out));
	out->minute     = 1UL << ((unsigned long)t->tm_min);
	out->hour       = 1 << t->tm_hour;
	out->dayofmonth = 1 << (t->tm_mday - 1);
	out->month      = 1 << t->tm_mon;
	out->dayofweek  = 1 << t->tm_wday;
}

bool cron_should_run(const crontab_t *t, const crontab_t *mask)
{
	if ((t->minute & mask->minute) == 0)
		return false;

	if ((t->hour & mask->hour) == 0)
		return false;

	if ((t->dayofmonth & mask->dayofmonth) == 0)
		return false;

	if ((t->month & mask->month) == 0)
		return false;

	if ((t->dayofweek & mask->dayofweek) == 0)
		return false;

	return true;
}

void delcron(crontab_t *cron)
{
	if (cron != NULL) {
		free(cron->exec);
		free(cron);
	}
}

int runjob(crontab_t *tab)
{
	pid_t pid;

	if (tab->exec == NULL)
		return 0;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}

	if (pid != 0)
		return 0;

	execl("/bin/sh", "sh", "-c", tab->exec, (char *) 0);
	perror("runnig shell interpreter");
	exit(EXIT_FAILURE);
}
