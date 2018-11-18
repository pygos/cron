/* SPDX-License-Identifier: ISC */
#include "gcrond.h"

int rdline(rdline_t *t)
{
	size_t i, len;

	do {
		free(t->line);
		t->line = NULL;
		errno = 0;
		len = 0;

		if (getline(&t->line, &len, t->fp) < 0) {
			if (errno) {
				rdline_complain(t, strerror(errno));
				return -1;
			}
			return 1;
		}

		t->lineno += 1;

		for (i = 0; isspace(t->line[i]); ++i)
			;

		if (t->line[i] == '\0' || t->line[i] == '#') {
			t->line[0] = '\0';
		} else if (i) {
			memmove(t->line, t->line + i, len - i + 1);
		}
	} while (t->line[0] == '\0');

	return 0;
}

void rdline_complain(rdline_t *t, const char *msg, ...)
{
	va_list ap;

	fprintf(stderr, "%s: %zu: ", t->filename, t->lineno);

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	fputc('\n', stderr);
}

int rdline_init(rdline_t *t, int dirfd, const char *filename)
{
	int fd;

	memset(t, 0, sizeof(*t));

	fd = openat(dirfd, filename, O_RDONLY);
	if (fd == -1) {
		perror(filename);
		return -1;
	}

	t->fp = fdopen(fd, "r");
	if (t->fp == NULL) {
		perror("fdopen");
		close(fd);
		return -1;
	}

	t->filename = filename;
	return 0;
}

void rdline_cleanup(rdline_t *t)
{
	free(t->line);
	fclose(t->fp);
}
