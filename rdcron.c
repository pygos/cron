/* SPDX-License-Identifier: ISC */
#include "gcrond.h"

typedef struct {
	const char *name;
	int value;
} enum_map_t;

static const enum_map_t weekday[] = {
	{ "MON", 1 },
	{ "TUE", 2 },
	{ "WED", 3 },
	{ "THU", 4 },
	{ "FRI", 5 },
	{ "SAT", 6 },
	{ "SUN", 0 },
	{ NULL, 0 },
};

static const enum_map_t month[] = {
	{ "JAN", 1 },
	{ "FEB", 2 },
	{ "MAR", 3 },
	{ "APR", 4 },
	{ "MAY", 5 },
	{ "JUN", 6 },
	{ "JUL", 7 },
	{ "AUG", 8 },
	{ "SEP", 9 },
	{ "OCT", 10 },
	{ "NOV", 11 },
	{ "DEC", 12 },
	{ NULL, 0 },
};

static const struct {
	const char *macro;
	crontab_t tab;
} intervals[] = {
	{
		.macro = "@yearly",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0x01,
			.month = 0x01,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "@annually",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0x01,
			.month = 0x01,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "@monthly",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0x01,
			.month = 0xFFFF,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "@weekly",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0xFFFFFFFF,
			.month = 0xFFFF,
			.dayofweek = 0x01
		},
	}, {
		.macro = "@daily",
		.tab = {
			.minute = 0x01,
			.hour = 0x01,
			.dayofmonth = 0xFFFFFFFF,
			.month = 0xFFFF,
			.dayofweek = 0xFF
		},
	}, {
		.macro = "@hourly",
		.tab = {
			.minute = 0x01,
			.hour = 0xFFFFFFFF,
			.dayofmonth = 0xFFFFFFFF,
			.month = 0xFFFF,
			.dayofweek = 0xFF
		},
	},
};

/*****************************************************************************/

static char *readnum(char *line, int *out, int minval, int maxval,
		     const enum_map_t *mnemonic, rdline_t *rd)
{
	int i, value = 0;
	const enum_map_t *ev;

	if (isalpha(line[0]) && mnemonic != NULL) {
		for (i = 0; isalpha(line[i]); ++i)
			;

		for (ev = mnemonic; ev->name != NULL; ++ev) {
			if (strncmp(line, ev->name, i) == 0 &&
			    ev->name[i] == '\0')
				break;
		}

		if (ev->name == NULL) {
			rdline_complain(rd, "unexpected '%.*s'", i, line);
			return NULL;
		}

		*out = ev->value;
		return line + i;
	}

	if (!isdigit(line[0]))
		goto fail_mn;

	while (isdigit(*line)) {
		if (value > INT_MAX / 10)
			goto fail_of;
		value = value * 10 + *(line++) - '0';
	}

	if (value > maxval)
		goto fail_of;
	if (value < minval)
		goto fail_uf;

	*out = value;
	return line;
fail_of:
	rdline_complain(rd, "value exceeds maximum (%d > %d)", value, maxval);
	return NULL;
fail_uf:
	rdline_complain(rd, "value too small (%d < %d)", value, minval);
	return NULL;
fail_mn:
	rdline_complain(rd, "expected numeric value");
	return NULL;
}

static char *readfield(char *line, uint64_t *out, int minval, int maxval,
		       const enum_map_t *mnemonic, rdline_t *rd)
{
	int value, endvalue, step;
	*out = 0;
next:
	if (*line == '*') {
		++line;
		value = minval;
		endvalue = maxval;
	} else {
		line = readnum(line, &value, minval, maxval, mnemonic, rd);
		if (!line)
			goto fail;

		if (*line == '-') {
			line = readnum(line + 1, &endvalue, minval, maxval,
				       mnemonic, rd);
			if (!line)
				goto fail;
		} else {
			endvalue = value;
		}

		if (endvalue < value)
			goto fail;
	}

	if (*line == '/') {
		line = readnum(line + 1, &step, 1, maxval + 1, NULL, rd);
		if (!line)
			goto fail;
	} else {
		step = 1;
	}

	while (value <= endvalue) {
		*out |= 1UL << (unsigned long)(value - minval);
		value += step;
	}

	if (*line == ',') {
		++line;
		goto next;
	}

	if (*line != '\0' && !isspace(*line))
		goto fail;
	while (isspace(*line))
		++line;

	return line;
fail:
	rdline_complain(rd, "invalid time range expression");
	return NULL;
}

/*****************************************************************************/

static char *cron_interval(crontab_t *cron, rdline_t *rd)
{
	size_t i, j;

	for (j = 1; isalpha(rd->line[j]); ++j)
		;
	if (j == 1 || !isspace(rd->line[j]))
		goto fail;

	for (i = 0; i < ARRAY_SIZE(intervals); ++i) {
		if (strlen(intervals[i].macro) != j)
			continue;
		if (strncmp(intervals[i].macro, rd->line, j) == 0)
			break;
	}

	if (i == ARRAY_SIZE(intervals))
		goto fail;

	*cron = intervals[i].tab;
	return rd->line + j;
fail:
	rdline_complain(rd, "unknown interval '%.*s'", (int)j, rd->line);
	return NULL;
}

static char *cron_fields(crontab_t *cron, rdline_t *rd)
{
	char *arg = rd->line;
	uint64_t value;

	if ((arg = readfield(arg, &value, 0, 59, NULL, rd)) == NULL)
		return NULL;
	cron->minute = value;

	if ((arg = readfield(arg, &value, 0, 23, NULL, rd)) == NULL)
		return NULL;
	cron->hour = value;

	if ((arg = readfield(arg, &value, 1, 31, NULL, rd)) == NULL)
		return NULL;
	cron->dayofmonth = value;

	if ((arg = readfield(arg, &value, 1, 12, month, rd)) == NULL)
		return NULL;
	cron->month = value;

	if ((arg = readfield(arg, &value, 0, 6, weekday, rd)) == NULL)
		return NULL;
	cron->dayofweek = value;

	return arg;
}

crontab_t *rdcron(int dirfd, const char *filename)
{
	crontab_t *cron, *list = NULL;
	rdline_t rd;
	char *ptr;

	if (rdline_init(&rd, dirfd, filename))
		return NULL;

	while (rdline(&rd) == 0) {
		cron = calloc(1, sizeof(*cron));
		if (cron == NULL) {
			rdline_complain(&rd, strerror(errno));
			break;
		}

		if (rd.line[0] == '@') {
			ptr = cron_interval(cron, &rd);
		} else {
			ptr = cron_fields(cron, &rd);
		}

		if (ptr == NULL) {
			free(cron);
			continue;
		}

		while (isspace(*ptr))
			++ptr;

		cron->exec = strdup(ptr);
		if (cron->exec == NULL) {
			rdline_complain(&rd, strerror(errno));
			free(cron);
			continue;
		}

		cron->next = list;
		list = cron;
	}

	rdline_cleanup(&rd);
	return list;
}
