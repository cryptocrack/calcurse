/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2023 calcurse Development Team <misc@calcurse.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>

#include "calcurse.h"

/* Input types for parse_datetimearg() */
enum {
	ARG_DATE,
	ARG_DATE_TIME,
	ARG_TIME,
	ARG_ERR
};

/* Long options */
enum {
	OPT_FILTER_INVERT = 1000,
	OPT_FILTER_TYPE,
	OPT_FILTER_HASH,
	OPT_FILTER_PATTERN,
	OPT_FILTER_START_FROM,
	OPT_FILTER_START_TO,
	OPT_FILTER_START_AFTER,
	OPT_FILTER_START_BEFORE,
	OPT_FILTER_START_RANGE,
	OPT_FILTER_END_FROM,
	OPT_FILTER_END_TO,
	OPT_FILTER_END_AFTER,
	OPT_FILTER_END_BEFORE,
	OPT_FILTER_END_RANGE,
	OPT_FILTER_PRIORITY,
	OPT_FILTER_COMPLETED,
	OPT_FILTER_UNCOMPLETED,
	OPT_FROM,
	OPT_TO,
	OPT_DAYS,
	OPT_FMT_APT,
	OPT_FMT_RAPT,
	OPT_FMT_EV,
	OPT_FMT_REV,
	OPT_FMT_TODO,
	OPT_DUMP_IMPORTED,
	OPT_EXPORT_UID,
	OPT_READ_ONLY,
	OPT_STATUS,
	OPT_DAEMON,
	OPT_INPUT_DATEFMT,
	OPT_OUTPUT_DATEFMT,
  OPT_PATTERN_ICASE
};

/*
 * Print Calcurse usage and exit.
 */
static void usage(void)
{
	printf("%s\n", _("Usage:\n"
			 "calcurse [-D <directory>] [-C <directory>] [-c <calendar file>]\n"
			 "calcurse -Q [--from <date>] [--to <date>] [--days <number>]\n"
			 "calcurse -a | -d <date> | -d <number> | -n | -r[<number>] | -s[<date>] | -t[<number>]\n"
			 "calcurse -h | -v | --status | -G | -P | -g | -i <file> | -x[<format>] | --daemon"));
}

static void usage_try(void)
{
	printf("%s\n", _("Try `calcurse -h` for more information."));
}

/*
 * Print Calcurse version with a short copyright text and exit.
 */
static void version_arg(void)
{
	printf(_("calcurse %s -- text-based organizer\n"), VERSION);
	putchar('\n');
	printf("%s\n", _("Copyright (c) 2004-2023 calcurse Development Team."));
	printf("%s\n", _("This is free software; see the source for copying conditions."));
}

/*
 * Print the command line options and exit.
 */
static void help_arg(void)
{
	usage();
	putchar('\n');
	printf("%s\n", _("Operations in command line mode:"));
	printf("%s\n", _("  -Q, --query   Print items in a given query range"));
	printf("%s\n", _("  -G, --grep    Grep items from the data files"));
	printf("%s\n", _("  -P, --purge   Read items and write them back"));
	printf("%s\n", _("Query short forms:\n"
			 "-a, -d <date>|<number>, -n, -r[<number>], -s[<date>], -t<number>"));
	putchar('\n');
	printf("%s\n", _("Note that filter, format and day-range options affect input or output:"));
	printf("%s\n", _("  --filter-*              Filter items loaded by -Q, -G, -P and -x"));
	printf("%s\n", _("  --format-*              Rewrite output from -Q, -G and --dump-imported"));
	printf("%s\n", _("  --from <date>           Limit day range of -Q."));
	printf("%s\n", _("  --to <date>             Limit day range of -Q."));
	printf("%s\n", _("  --days <number>         Limit day range of -Q."));
	putchar('\n');
	printf("%s\n", _("  --limit, -l <number>    Limit number of query results"));
	printf("%s\n", _("  --search, -S <regexp>   Match regular expression in queries"));
	printf("%s\n", _("Consult the man page for details."));
	putchar('\n');
	printf("%s\n", _("Miscellaneous:"));
	printf("%s\n", _("  -c, --calendar <file>   The calendar data file to use"));
	printf("%s\n", _("  -C, --confdir <dir>     The configuration directory to use"));
	printf("%s\n", _("  --daemon                Run notification daemon in the background"));
	printf("%s\n", _("  -D, --datadir <dir>     The data directory to use"));
	printf("%s\n", _("  -g, --gc                Run the garbage collector"));
	printf("%s\n", _("  -h, --help              Show this help text"));
	printf("%s\n", _("  -i, --import <file>     Import iCal data from file"));
	printf("%s\n", _("  -q, --quiet             Suppress import/export result message"));
	printf("%s\n", _("  --read-only             Do not save configuration or data files"));
	printf("%s\n", _("  --status                Display status of running instances"));
	printf("%s\n", _("  -v, --version           Show version information"));
	printf("%s\n", _("  -x, --export[<format>]  Export to stdout in ical (default) or pcal format"));
	putchar('\n');
	printf("%s\n", _("For more information, type '?' from within calcurse, or read the manpage."));
	printf("%s\n", _("Submit feature requests and suggestions to <misc@calcurse.org>."));
	printf("%s\n", _("Submit bug reports to <bugs@calcurse.org>."));
}

/*
 * Used to display the status of running instances of calcurse.
 * The displayed message will look like one of the following ones:
 *
 *   calcurse is running (pid #)
 *   calcurse is running in background (pid #)
 *   calcurse is not running
 *
 * The status is obtained by looking at pid files in user data directory
 * (.calcurse.pid and .daemon.pid).
 */
static void status_arg(void)
{
	int cpid, dpid;

	cpid = io_get_pid(path_cpid);
	dpid = io_get_pid(path_dpid);

	EXIT_IF(cpid && dpid,
		_("Error: both calcurse (pid: %d) and its daemon (pid: %d)\n"
		 "seem to be running at the same time!\n"
		 "Please check manually and restart calcurse.\n"), cpid,
		dpid);

	if (cpid)
		fprintf(stdout, _("calcurse is running (pid %d)\n"), cpid);
	else if (dpid)
		fprintf(stdout,
			_("calcurse is running in background (pid %d)\n"),
			dpid);
	else
		puts(_("calcurse is not running"));
}

/* Print TODO list and return the number of printed items. */
static int todo_arg(const char *format, int *limit, struct item_filter *filter)
{
	const char *titlestr =
		filter->completed ? _("completed tasks:\n") : _("to do:\n");
	int title = 1;
	int n = 0;
	llist_item_t *i;

	LLIST_FOREACH(&todolist, i) {
		if (*limit == 0)
			break;
		struct todo *todo = LLIST_TS_GET_DATA(i);

		if (title) {
			fputs(titlestr, stdout);
			title = 0;
		}
		print_todo(format, todo);
		n++;
		(*limit)--;
	}

	return n;
}

/* Print the next appointment within the upcoming 24 hours. */
static void next_arg(void)
{
	struct notify_app next_app;
	const long current_time = now();
	int time_left, hours_left, min_left;

	next_app.time = date_sec_change(current_time, 0, 1);
	next_app.got_app = 0;
	next_app.txt = NULL;

	recur_apoint_check_next(&next_app, current_time, get_today());
	next_app = *apoint_check_next(&next_app, current_time);

	if (next_app.got_app) {
		time_left = next_app.time - current_time;
		/* In minutes rounded up. */
		min_left = time_left / MININSEC +
			   (time_left % MININSEC ? 1 : 0);
		hours_left = min_left / HOURINMIN;
		min_left = min_left % HOURINMIN;
		fputs(_("next appointment:\n"), stdout);
		fprintf(stdout, "   [%02d:%02d] %s\n", hours_left,
			min_left, next_app.txt);
		mem_free(next_app.txt);
	}
}

/*
 * Print the date on stdout.
 */
static void arg_print_date(long date)
{
	char date_str[BUFSIZ];
	struct tm lt;

	localtime_r((time_t *) & date, &lt);
	strftime(date_str, BUFSIZ, conf.output_datefmt, &lt);
	fputs(date_str, stdout);
	fputs(":\n", stdout);
}

/*
 * Print appointments inside the given query range.
 * If no start day is given (-1), today is considered.
 * If no end date is given (-1), a range of 1 day is considered.
 */
static void
date_arg_from_to(long from, long to, int add_line, const char *fmt_apt,
		 const char *fmt_rapt, const char *fmt_ev, const char *fmt_rev,
		 int *limit)
{
	long date;

	for (date = from; date <= to; date = date_sec_change(date, 0, 1)) {
		day_store_items(date, 0, 1);
		if (day_item_count(0) == 0)
			continue;
		if (add_line)
			fputs("\n", stdout);
		arg_print_date(date);
		day_write_stdout(date, fmt_apt, fmt_rapt, fmt_ev,
				 fmt_rev, limit);
		add_line = 1;
	}
}

/*
 * Convert a string with a (local time) date, date-time or time into
 * the Unix time for that point in time as follows:
 * - a date only is converted to midnight (beginning) of that day
 * - a date-time is converted to that day and time
 * - a time only is converted to that time of the current day
 * The date format is taken from the user configuration.
 * The type of the input string is returned in the type argument.
 */

static time_t parse_datetimearg(const char *str, int *type)
{
	char *date = mem_strdup(str);
	*type = ARG_ERR;
	char *time;
	struct date day;
	unsigned hour, min;
	time_t ret;

	time = strchr(date, ' ');
	if (time) { /* Date and time? */
		*time++ = '\0';
		if (!parse_time(time, &hour, &min))
			return -1;
		if (!parse_date(date, conf.input_datefmt,
		    (int *)&day.yyyy, (int *)&day.mm, (int *)&day.dd, NULL))
			return -1;
		ret = date2sec(day, hour, min);
		*type = ARG_DATE_TIME;
	} else /* Date?*/ if (parse_date(date, conf.input_datefmt,
		    (int *)&day.yyyy, (int *)&day.mm, (int *)&day.dd, NULL)) {
		ret = date2sec(day, 0, 0);
		*type = ARG_DATE;
	} else /* Time? */ if (parse_time(date, &hour, &min)) {
		ret = date2sec(*ui_calendar_get_today(), hour, min);
		*type = ARG_TIME;
	} else
		return -1;
	return ret;
}

/*
 * Parse a "from,to" date range.
 * For an open-end range no change occurs at the open end.
 */
static int parse_daterange(const char *str, time_t *date_from, time_t *date_to)
{
	int type, ret = 0;
	char *s = mem_strdup(str);
	char *p = strchr(s, ',');

	if (!p)
		goto cleanup;

	*p = '\0';
	p++;

	if (*s != '\0') {
		*date_from = parse_datetimearg(s, &type);
		if (*date_from == -1)
			goto cleanup;
	}

	if (*p != '\0') {
		*date_to = parse_datetimearg(p, &type);
		if (*date_to == -1)
			goto cleanup;
		if (type == ARG_DATE)
			*date_to = ENDOFDAY(*date_to);
	}

	ret = 1;
cleanup:
	mem_free(s);
	return ret;
}

static int parse_type_mask(const char *str)
{
	char *buf = mem_strdup(str), *p;
	int mask = 0;

	for (p = strtok(buf, ","); p; p = strtok(NULL, ",")) {
		if (!strcmp(p, "event")) {
			mask |= TYPE_MASK_EVNT;
		} else if (!strcmp(p, "apt")) {
			mask |= TYPE_MASK_APPT;
		} else if (!strcmp(p, "recur-event")) {
			mask |= TYPE_MASK_RECUR_EVNT;
		} else if (!strcmp(p, "recur-apt")) {
			mask |= TYPE_MASK_RECUR_APPT;
		} else if (!strcmp(p, "recur")) {
			mask |= TYPE_MASK_RECUR;
		} else if (!strcmp(p, "cal")) {
			mask |= TYPE_MASK_CAL;
		} else if (!strcmp(p, "todo")) {
			mask |= TYPE_MASK_TODO;
		} else {
			mask = 0;
			goto cleanup;
		}
	}

cleanup:
	mem_free(buf);
	return mask;
}

/*
 * Parse the command-line arguments and call the appropriate
 * routines to handle those arguments. Also initialize the data paths.
 * Exit here in case of errors else return the non-interactive value.
 */
int parse_args(int argc, char **argv)
{
	/* Command-line flags - NOTE that read_only is global */
	int grep = 0, grep_filter = 0, purge = 0, query = 0, next = 0;
	int status = 0, gc = 0, import = 0, export = 0, daemon = 0;
	/* Command line invocation */
	int filter_opt = 0, format_opt = 0, query_range = 0, cmd_line = 0;
	int start_from = 0, start_to = 0, end_from = 0, end_to = 0;
	/* Query ranges */
	time_t from = -1, to = -1;
	int range = 0;
	int limit = INT_MAX;
  int flag_case_insensitive = 0;
	/* Filters */
	struct item_filter filter = { 0, 0, NULL, NULL, -1, -1, -1, -1, 0, 0, 0 };
	/* Format strings */
	const char *fmt_apt = NULL;
	const char *fmt_rapt = NULL;
	const char *fmt_ev = NULL;
	const char *fmt_rev = NULL;
	const char *fmt_todo = NULL;
	/* Import and export parameters */
	int xfmt = IO_EXPORT_ICAL;
	int dump_imported = 0, export_uid = 0;
	/* Data file locations */
	const char *datadir = NULL;
	const char *cfile = NULL, *confdir = NULL;
	char *ifile = NULL;

	int ret, non_interactive = 1;
	int ch, cpid, type;
	regex_t reg;
	char buf[BUFSIZ];
	struct tm tm;
	time_t t;

	static const char *optstr = "PFgGhvnNax::t::C:d:c:r::s::S:D:i:l:qQ";

	struct option longopts[] = {
		{"appointment", no_argument, NULL, 'a'},
		{"calendar", required_argument, NULL, 'c'},
		/* Deprecated */
		{"conf", required_argument, NULL, 'C'},
		{"confdir", required_argument, NULL, 'C'},
		{"day", required_argument, NULL, 'd'},
		{"datadir", required_argument, NULL, 'D'},
		/* Deprecated */
		{"directory", required_argument, NULL, 'D'},
		{"filter", no_argument, NULL, 'F'},
		{"gc", no_argument, NULL, 'g'},
		{"grep", no_argument, NULL, 'G'},
		{"help", no_argument, NULL, 'h'},
		{"import", required_argument, NULL, 'i'},
		{"limit", required_argument, NULL, 'l'},
		{"next", no_argument, NULL, 'n'},
		{"note", no_argument, NULL, 'N'},
		{"purge", no_argument, NULL, 'P'},
		{"range", optional_argument, NULL, 'r'},
		{"startday", optional_argument, NULL, 's'},
		{"search", required_argument, NULL, 'S'},
		{"todo", optional_argument, NULL, 't'},
		{"version", no_argument, NULL, 'v'},
		{"export", optional_argument, NULL, 'x'},
		{"quiet", no_argument, NULL, 'q'},
		{"query", optional_argument, NULL, 'Q'},

		{"filter-invert", no_argument, NULL, OPT_FILTER_INVERT},
		{"filter-type", required_argument, NULL, OPT_FILTER_TYPE},
		{"filter-hash", required_argument, NULL, OPT_FILTER_HASH},
		{"filter-pattern", required_argument, NULL, OPT_FILTER_PATTERN},
		{"filter-start-from", required_argument, NULL, OPT_FILTER_START_FROM},
		{"filter-start-to", required_argument, NULL, OPT_FILTER_START_TO},
		{"filter-start-after", required_argument, NULL, OPT_FILTER_START_AFTER},
		{"filter-start-before", required_argument, NULL, OPT_FILTER_START_BEFORE},
		{"filter-start-range", required_argument, NULL, OPT_FILTER_START_RANGE},
		{"filter-end-from", required_argument, NULL, OPT_FILTER_END_FROM},
		{"filter-end-to", required_argument, NULL, OPT_FILTER_END_TO},
		{"filter-end-after", required_argument, NULL, OPT_FILTER_END_AFTER},
		{"filter-end-before", required_argument, NULL, OPT_FILTER_END_BEFORE},
		{"filter-end-range", required_argument, NULL, OPT_FILTER_END_RANGE},
		{"filter-priority", required_argument, NULL, OPT_FILTER_PRIORITY},
		{"filter-completed", no_argument, NULL, OPT_FILTER_COMPLETED},
		{"filter-uncompleted", no_argument, NULL, OPT_FILTER_UNCOMPLETED},
		{"from", required_argument, NULL, OPT_FROM},
		{"to", required_argument, NULL, OPT_TO},
		{"days", required_argument, NULL, OPT_DAYS},
		{"format-apt", required_argument, NULL, OPT_FMT_APT},
		{"format-recur-apt", required_argument, NULL, OPT_FMT_RAPT},
		{"format-event", required_argument, NULL, OPT_FMT_EV},
		{"format-recur-event", required_argument, NULL, OPT_FMT_REV},
		{"format-todo", required_argument, NULL, OPT_FMT_TODO},
		{"export-uid", no_argument, NULL, OPT_EXPORT_UID},
		{"dump-imported", no_argument, NULL, OPT_DUMP_IMPORTED},
		{"read-only", no_argument, NULL, OPT_READ_ONLY},
		{"status", no_argument, NULL, OPT_STATUS},
		{"daemon", no_argument, NULL, OPT_DAEMON},
		{"input-datefmt", required_argument, NULL, OPT_INPUT_DATEFMT},
		{"output-datefmt", required_argument, NULL, OPT_OUTPUT_DATEFMT},
		{"pattern-case-insensitive", no_argument, NULL, OPT_PATTERN_ICASE},
		{NULL, no_argument, NULL, 0}
	};

	/*
	 * Load the configuration file first to get the input date format for
	 * parsing the remaining options.
	 */
	while ((ch = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (ch) {
		case 'C':
			confdir = optarg;
			break;
		case 'D':
			datadir = optarg;
			break;
		case 'c':
			cfile = optarg;
			break;
		case '?':
			usage();
			usage_try();
			exit(EXIT_FAILURE);                                             \
		}
	}
	io_init(cfile, datadir, confdir);
	vars_init();
	notify_init_vars();
	if (io_file_exists(path_conf))
		config_load();

	/* Parse the remaining options. */
	optind = 1;
	while ((ch = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (ch) {
		case 'a':
			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 'c':
			break;
		case 'C':
			break;
		case 'd':
			if (is_all_digit(optarg) ||
			    (*optarg == '-' && is_all_digit(optarg + 1))) {
				range = atoi(optarg);
				EXIT_IF(range == 0, _("invalid range: %s"),
					optarg);
			} else {
				from = parse_datetimearg(optarg, &type);
				EXIT_IF(from == -1 || type != ARG_DATE,
					_("invalid date: %s"), optarg);
			}

			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 'D':
			break;
		case 'F':
			grep_filter = grep = 1;
			break;
		case 'h':
			help_arg();
			exit(EXIT_SUCCESS);
		case 'g':
			gc = 1;
			break;
		case 'G':
			grep = 1;
			break;
		case 'i':
			import = 1;
			ifile = optarg;
			break;
		case 'l':
			limit = atoi(optarg);
			break;
		case 'n':
			next = 1;
			break;
		case 'P':
			filter.invert = 1;
			purge = grep = 1;
			break;
		case 'r':
			if (optarg)
				range = atoi(optarg);
			else
				range = 1;
			EXIT_IF(range == 0, _("invalid range: %s"), optarg);
			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 's':
			if (!optarg)
				optarg = "today";
			from = parse_datetimearg(optarg, &type);
			EXIT_IF(from == -1 || type != ARG_DATE,
				_("invalid date: %s"), optarg);
			filter.type_mask |= TYPE_MASK_CAL;
			query = 1;
			break;
		case 't':
			if (optarg) {
				EXIT_IF(!is_all_digit(optarg),
					_("invalid priority: %s"), optarg);
				filter.priority = atoi(optarg);
				if (filter.priority == 0)
					filter.completed = 1;
				else
					filter.uncompleted = 1;
				EXIT_IF(filter.priority > 9,
					_("invalid priority: %s"), optarg);
			} else {
				filter.uncompleted = 1;
			}
			filter.type_mask |= TYPE_MASK_TODO;
			query = 1;
			break;
		case 'v':
			version_arg();
			exit(EXIT_SUCCESS);
		case 'x':
			export = 1;
			if (optarg) {
				if (!strcmp(optarg, "ical"))
					xfmt = IO_EXPORT_ICAL;
				else if (!strcmp(optarg, "pcal"))
					xfmt = IO_EXPORT_PCAL;
				else
					EXIT(_("invalid export format: %s"),
					     optarg);
			}
			break;
		case 'q':
			quiet = 1;
			break;
		case 'Q':
			query = 1;
			break;
		case OPT_FILTER_INVERT:
			filter.invert = !filter.invert;
			filter_opt = 1;
			break;
		case OPT_FILTER_TYPE:
			filter.type_mask = parse_type_mask(optarg);
			EXIT_IF(filter.type_mask == 0,
				_("invalid filter mask"));
			filter_opt = 1;
			break;
		case OPT_FILTER_HASH:
			filter.hash = mem_strdup(optarg);
			filter_opt = 1;
			break;
		case OPT_PATTERN_ICASE:
      flag_case_insensitive = REG_ICASE;
			break;
		case 'S':
		case OPT_FILTER_PATTERN:
			EXIT_IF(filter.regex,
				_("cannot handle more than one regular expression"));
			if (regcomp(&reg, optarg, REG_EXTENDED | flag_case_insensitive))
				EXIT(_("could not compile regular expression: %s"), optarg);
			filter.regex = &reg;
			filter_opt = 1;
			break;
		/*
		 * A date only is a time span (a day) and interpreted thus:
		 * "to" means "to end of day"
		 * "after" means "from start of next day"
		 */
		case OPT_FILTER_START_FROM:
			EXIT_IF(start_from,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.start_from = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.start_from == -1,
				_("invalid date: %s"), optarg);
			start_from = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_START_TO:
			EXIT_IF(start_to,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.start_to = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.start_to == -1,
				_("invalid date: %s"), optarg);
			if (type == ARG_DATE)
				filter.start_to = ENDOFDAY(filter.start_to);
			start_to = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_START_AFTER:
			EXIT_IF(start_from,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.start_from = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.start_from == -1,
				_("invalid date: %s"), optarg);
			if (type == ARG_DATE)
				filter.start_from = NEXTDAY(filter.start_from);
			else
				filter.start_from++;
			start_from = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_START_BEFORE:
			EXIT_IF(start_to,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.start_to = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.start_to == -1,
				_("invalid date: %s"), optarg);
			filter.start_to--;
			start_to = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_START_RANGE:
			EXIT_IF(start_from,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			EXIT_IF(start_to,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			/* Set initialization values in case of open-end range. */
			filter.start_from = filter.start_to = -1;
			EXIT_IF(!parse_daterange(optarg, &filter.start_from, &filter.start_to),
				_("invalid date range: %s"), optarg);
			start_from = 1;
			start_to = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_END_FROM:
			EXIT_IF(end_from,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.end_from = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.end_from == -1,
				_("invalid date: %s"), optarg);
			end_from = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_END_TO:
			EXIT_IF(end_to,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.end_to = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.end_to == -1,
				_("invalid date: %s"), optarg);
			if (type == ARG_DATE)
				filter.end_to = ENDOFDAY(filter.end_to);
			end_to = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_END_AFTER:
			EXIT_IF(end_from,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.end_from = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.end_from == -1,
				_("invalid date: %s"), optarg);
			if (type == ARG_DATE)
				filter.end_from = NEXTDAY(filter.end_from);
			else
				filter.end_from++;
			end_from = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_END_BEFORE:
			EXIT_IF(end_to,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			filter.end_to = parse_datetimearg(optarg, &type);
			EXIT_IF(filter.end_to == -1,
				_("invalid date: %s"), optarg);
			filter.end_to--;
			end_to = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_END_RANGE:
			EXIT_IF(end_from,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			EXIT_IF(end_to,
			    _("filter criterion already in use: %s"), argv[optind - 2]);
			/* Set default values in case of open-ended range. */
			filter.start_from = filter.start_to = -1;
			EXIT_IF(!parse_daterange(optarg, &filter.end_from, &filter.end_to),
				_("invalid date range: %s"), optarg);
			end_from = 1;
			end_to = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_PRIORITY:
			filter.priority = atoi(optarg);
			EXIT_IF(filter.priority < 1 || filter.priority > 9,
				_("invalid priority: %s"), optarg);
			filter_opt = 1;
			break;
		case OPT_FILTER_COMPLETED:
			filter.completed = 1;
			filter_opt = 1;
			break;
		case OPT_FILTER_UNCOMPLETED:
			filter.uncompleted = 1;
			filter_opt = 1;
			break;
		case OPT_FROM:
			from = parse_datetimearg(optarg, &type);
			EXIT_IF(from == -1 || type != ARG_DATE,
				_("invalid date: %s"), optarg);
			query_range = 1;
			break;
		case OPT_TO:
			to = parse_datetimearg(optarg, &type);
			EXIT_IF(to == -1 || type != ARG_DATE,
				_("invalid date: %s"), optarg);
			query_range = 1;
			break;
		case OPT_DAYS:
			range = atoi(optarg);
			EXIT_IF(range == 0, _("invalid range: %s"), optarg);
			query_range = 1;
			break;
		case OPT_FMT_APT:
			fmt_apt = optarg;
			format_opt = 1;
			break;
		case OPT_FMT_RAPT:
			fmt_rapt = optarg;
			format_opt = 1;
			break;
		case OPT_FMT_EV:
			fmt_ev = optarg;
			format_opt = 1;
			break;
		case OPT_FMT_REV:
			fmt_rev = optarg;
			format_opt = 1;
			break;
		case OPT_FMT_TODO:
			fmt_todo = optarg;
			format_opt = 1;
			break;
		case OPT_DUMP_IMPORTED:
			dump_imported = 1;
			break;
		case OPT_EXPORT_UID:
			export_uid = 1;
			break;
		case OPT_READ_ONLY:
			read_only = 1;
			break;
		case OPT_STATUS:
			status = 1;
			break;
		case OPT_DAEMON:
			EXIT_IF(cpid = io_get_pid(path_cpid),
				_("calcurse is running (pid = %d)"), cpid);
			daemon = 1;
			filter.type_mask = TYPE_MASK_APPT | TYPE_MASK_RECUR_APPT;
			break;
		case OPT_INPUT_DATEFMT:
			conf.input_datefmt = atoi(optarg);
			EXIT_IF(conf.input_datefmt < 1 || conf.input_datefmt > 4,
				_("invalid input date format: %s"), optarg);
			cmd_line = 1;
			break;
		case OPT_OUTPUT_DATEFMT:
			time(&t);
			localtime_r(&t, &tm);
			EXIT_IF(!strftime(buf, sizeof(buf), optarg, &tm),
			       _("invalid output date format: %s"), optarg);
			strncpy(conf.output_datefmt, optarg,
				sizeof(conf.output_datefmt) - 1);
			conf.output_datefmt[sizeof(conf.output_datefmt) - 1] =
				'\0';
			cmd_line = 1;
			break;
		}
	}

	if (filter.type_mask == 0)
		filter.type_mask = TYPE_MASK_ALL;

	if (status + grep + query + next + gc + import + export + daemon > 1 ||
	    optind < argc ||
	    (filter_opt && !(grep + query + export)) ||
	    (format_opt && !(grep + query + dump_imported)) ||
	    (query_range && !query) ||
	    (purge && !filter.invert)
	   )
		EXIT(_("invalid argument combination"));

	EXIT_IF(to >= 0 && range, _("cannot specify a range and an end date"));
	if (from == -1)
		from = get_today();
	if (to == -1)
		to = ENDOFDAY(from);
	EXIT_IF(to < from, _("end date cannot come before start date"));
	if (range > 0)
		to = date_sec_change(from, 0, range - 1);
	else if (range < 0)
		from = date_sec_change(to, 0, range + 1);

	io_check_dir(path_ddir);
	io_check_dir(path_notes);
	io_check_dir(path_cdir);
	io_check_dir(path_hooks);

	if (status) {
		status_arg();
	} else if (grep) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_check_file(path_conf);
		io_load_data(&filter, FORCE);
		if (purge || grep_filter) {
			io_save_todo(path_todo);
			io_save_apts(path_apts);
		} else {
			/*
			 * Use default values for non-specified format strings.
			 */
			fmt_apt = fmt_apt ? fmt_apt : "%(raw)";
			fmt_rapt = fmt_rapt ? fmt_rapt : "%(raw)";
			fmt_ev = fmt_ev ? fmt_ev : "%(raw)";
			fmt_rev = fmt_rev ? fmt_rev : "%(raw)";
			fmt_todo = fmt_todo ? fmt_todo : "%(raw)";

			io_dump_todo(fmt_todo);
			io_dump_apts(fmt_apt, fmt_rapt, fmt_ev, fmt_rev);
		}
	} else if (query) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_check_file(path_conf);
		io_load_data(&filter, FORCE);

		/* Use default values for non-specified format strings. */
		fmt_apt = fmt_apt ? fmt_apt : " - %S -> %E\n\t%m\n";
		fmt_rapt = fmt_rapt ? fmt_rapt : " - %S -> %E\n\t%m\n";
		fmt_ev = fmt_ev ? fmt_ev : " * %m\n";
		fmt_rev = fmt_rev ? fmt_rev : " * %m\n";
		fmt_todo = fmt_todo ? fmt_todo : "%p. %m\n";

		int add_line = todo_arg(fmt_todo, &limit, &filter);
		date_arg_from_to(from, to, add_line, fmt_apt, fmt_rapt, fmt_ev,
				 fmt_rev, &limit);
	} else if (next) {
		io_check_file(path_apts);
		io_load_app(&filter);
		next_arg();
	} else if (gc) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_load_data(NULL, FORCE);
		note_gc();
	} else if (import) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_load_data(NULL, FORCE);
		if (dump_imported) {
			/*
			 * Use default values for non-specified format strings.
			 */
			fmt_apt = fmt_apt ? fmt_apt : "%(raw)";
			fmt_rapt = fmt_rapt ? fmt_rapt : "%(raw)";
			fmt_ev = fmt_ev ? fmt_ev : "%(raw)";
			fmt_rev = fmt_rev ? fmt_rev : "%(raw)";
			fmt_todo = fmt_todo ? fmt_todo : "%(raw)";
		} else {
			/*
			 * Do not dump items, unset format strings explicitly.
			 */
			fmt_apt = fmt_rapt = fmt_ev = fmt_rev = NULL;
			fmt_todo = NULL;
		}
		ret = io_import_data(IO_IMPORT_ICAL, ifile, fmt_ev, fmt_rev,
				     fmt_apt, fmt_rapt, fmt_todo);
		io_save_apts(path_apts);
		io_save_todo(path_todo);
		if (!ret)
			exit_calcurse(EXIT_FAILURE);
	} else if (export) {
		io_check_file(path_apts);
		io_check_file(path_todo);
		io_load_data(&filter, FORCE);
		io_export_data(xfmt, export_uid);
	} else if (daemon) {
		dmon_stop();
		dmon_start(0);
	} else if (!cmd_line) {
		/* interactive mode */
		non_interactive = 0;
	}

	/* Free filter parameters. */
	if (filter.regex)
		regfree(filter.regex);

	return non_interactive;
}
