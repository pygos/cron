/* SPDX-License-Identifier: ISC */
#include "gcrond.h"

static crontab_t *jobs;
static sig_atomic_t run = 1;
static sig_atomic_t rescan = 1;

static void read_config(void)
{
	if (cronscan(GCRONDIR, &jobs)) {
		fputs("Error reading configuration. Continuing anyway.\n",
		      stderr);
	}
}

static void cleanup_config(void)
{
	crontab_t *t;

	while (jobs != NULL) {
		t = jobs;
		jobs = jobs->next;
		delcron(t);
	}
}

static int timeout_minutes(int minutes)
{
	time_t now = time(NULL);
	struct tm t;

	localtime_r(&now, &t);
	return minutes * 60 + 30 - t.tm_sec;
}

static int calc_timeout(void)
{
	time_t now = time(NULL), future;
	struct tm tmstruct;
	crontab_t mask, *t;
	int minutes;

	for (minutes = 0; minutes < 120; ++minutes) {
		future = now + minutes * 60;

		localtime_r(&future, &tmstruct);
		cron_tm_to_mask(&mask, &tmstruct);

		for (t = jobs; t != NULL; t = t->next) {
			if (cron_should_run(t, &mask))
				goto out;
		}
	}
out:
	return timeout_minutes(minutes ? minutes : 1);
}

static void runjobs(void)
{
	time_t now = time(NULL);
	struct tm tmstruct;
	crontab_t mask, *t;

	localtime_r(&now, &tmstruct);
	cron_tm_to_mask(&mask, &tmstruct);

	for (t = jobs; t != NULL; t = t->next) {
		if (cron_should_run(t, &mask))
			runjob(t);
	}
}

static void sighandler(int signo)
{
	pid_t pid;

	switch (signo) {
	case SIGINT:
	case SIGTERM:
		run = 0;
		break;
	case SIGHUP:
		rescan = 1;
		break;
	case SIGCHLD:
		while ((pid = waitpid(-1, NULL, WNOHANG)) != -1)
			;
		break;
	}
}

int main(void)
{
	struct timespec stime;
	struct sigaction act;
	int timeout;

	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);

	while (run) {
		if (rescan == 1) {
			cleanup_config();
			read_config();
			timeout = timeout_minutes(1);
			rescan = 0;
		} else {
			runjobs();
			timeout = calc_timeout();
		}

		stime.tv_sec = timeout;
		stime.tv_nsec = 0;

		while (nanosleep(&stime, &stime) != 0 && run && !rescan) {
			if (errno != EINTR) {
				perror("nanosleep");
				break;
			}
		}
	}

	return EXIT_SUCCESS;
}
