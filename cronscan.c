/* SPDX-License-Identifier: ISC */
#include "gcrond.h"

int cronscan(const char *directory, crontab_t **list)
{
	crontab_t *cron, *tail = NULL;
	struct dirent *ent;
	int dfd, ret = 0;
	DIR *dir;

	dir = opendir(directory);
	if (dir == NULL) {
		perror(directory);
		return -1;
	}

	dfd = dirfd(dir);
	if (dfd < 0) {
		perror(directory);
		closedir(dir);
		return -1;
	}

	for (;;) {
		errno = 0;
		ent = readdir(dir);

		if (ent == NULL) {
			if (errno != 0) {
				perror(directory);
				ret = -1;
			}
			break;
		}

		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;

		cron = rdcron(dfd, ent->d_name);
		if (cron == NULL)
			continue;

		if (tail == NULL) {
			*list = cron;
			tail = cron;
		} else {
			tail->next = cron;
		}

		while (tail->next != NULL)
			tail = tail->next;
	}

	closedir(dir);
	return ret;
}
