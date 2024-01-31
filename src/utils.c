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

#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <termios.h>

#include "calcurse.h"
#include "sha1.h"

#define FS_EXT_MAXLEN 64

enum format_specifier {
	FS_STARTDATE,
	FS_DURATION,
	FS_ENDDATE,
	FS_REMAINING,
	FS_MESSAGE,
	FS_NOTE,
	FS_NOTEFILE,
	FS_PRIORITY,
	FS_RAW,
	FS_HASH,
	FS_PSIGN,
	FS_EOF,
	FS_UNKNOWN
};

/* General routine to exit calcurse properly. */
void exit_calcurse(int status)
{
	int was_interactive;

	if (ui_mode == UI_CURSES) {
		notify_stop_main_thread();
		ui_calendar_stop_date_thread();
		io_stop_psave_thread();

		clear();
		wins_refresh();
		endwin();
		ui_mode = UI_CMDLINE;
		was_interactive = 1;
	} else {
		was_interactive = 0;
	}

	free_user_data();
	keys_free();
	mem_stats();

	if (was_interactive) {
		if (unlink(path_cpid) != 0)
			EXIT(_("Could not remove calcurse lock file: %s\n"),
			     strerror(errno));
		if (dmon.enable)
			dmon_start(status);
	}

	exit(status);
}

void free_user_data(void)
{
	unsigned i;

	day_free_vector();
	event_llist_free();
	apoint_llist_free();
	recur_apoint_llist_free();
	recur_event_llist_free();
	for (i = 0; i <= REG_BLACK_HOLE; i++)
		ui_day_item_cut_free(i);
	todo_free_list();
	notify_free_app();
}

/* Function to exit on internal error. */
void fatalbox(const char *errmsg)
{
	WINDOW *errwin;
	const char *label = _("/!\\ INTERNAL ERROR /!\\");
	const char *reportmsg = _("Please report the following bug:");
	const int WINROW = 10;
	const int WINCOL = col - 2;
	const int MSGLEN = WINCOL - 2;
	char msg[MSGLEN];

	if (errmsg == NULL)
		return;

	strncpy(msg, errmsg, MSGLEN);
	msg[MSGLEN - 1] = '\0';
	errwin =
	    newwin(WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2);
	custom_apply_attr(errwin, ATTR_HIGHEST);
	box(errwin, 0, 0);
	wins_show(errwin, label);
	mvwaddstr(errwin, 3, 1, reportmsg);
	mvwaddstr(errwin, 5, (WINCOL - strlen(msg)) / 2, msg);
	custom_remove_attr(errwin, ATTR_HIGHEST);
	wins_wrefresh(errwin);
	keys_wait_for_any_key(errwin);
	delwin(errwin);
	wins_doupdate();
}

void warnbox(const char *msg)
{
	WINDOW *warnwin;
	const char *label = "/!\\";
	const int WINROW = 10;
	const int WINCOL = col - 2;
	const int MSGLEN = WINCOL - 2;
	char displmsg[MSGLEN];

	if (msg == NULL)
		return;

	strncpy(displmsg, msg, MSGLEN);
	displmsg[MSGLEN - 1] = '\0';
	warnwin =
	    newwin(WINROW, WINCOL, (row - WINROW) / 2, (col - WINCOL) / 2);
	custom_apply_attr(warnwin, ATTR_HIGHEST);
	box(warnwin, 0, 0);
	wins_show(warnwin, label);
	mvwaddstr(warnwin, 5, (WINCOL - strlen(displmsg)) / 2, displmsg);
	custom_remove_attr(warnwin, ATTR_HIGHEST);
	wins_wrefresh(warnwin);
	keys_wait_for_any_key(warnwin);
	delwin(warnwin);
	wins_doupdate();
}

/*
 * Print a message in the status bar.
 * Message texts for first line and second line are to be provided.
 */
void status_mesg(const char *msg1, const char *msg2)
{
	wins_erase_status_bar();
	custom_apply_attr(win[STA].p, ATTR_HIGHEST);
	mvwaddstr(win[STA].p, 0, 0, msg1);
	mvwaddstr(win[STA].p, 1, 0, msg2);
	custom_remove_attr(win[STA].p, ATTR_HIGHEST);
	wins_wrefresh(win[STA].p);
}

/*
 * Prompts the user to make a choice between named alternatives.
 *
 * The available choices are described (in po-files) by a string of the form
 * "[ynp]". The first and last char are ignored (they are only here to
 * make the translators' life easier), and every other char indicates
 * a key the user is allowed to press.
 *
 * Returns the index of the key pressed by the user (starting from 1),
 * or -1 if the user doesn't want to answer (e.g. by escaping).
 */
int status_ask_choice(const char *message, const char choice[],
		      int nb_choice)
{
	/* Turn "[42w...Z]" into * "[4/2/w/.../Z]". */
	char avail_choice[nb_choice * UTF8_MAXLEN + nb_choice + 1];
	int ichoice[nb_choice];
	int i, j, k, n, ch;

	avail_choice[0] = '[';
	for (n = 0, i = 1, j = 1; n < nb_choice; n++, i += k) {
		for (k = 0; k < UTF8_LENGTH(choice[i]); k++) {
			avail_choice[j] = choice[i + k];
			j++;
		}
		avail_choice[j] = '/';
		j++;
	}
	avail_choice[j - 1] = ']';
	avail_choice[j] = '\0';

	status_mesg(message, avail_choice);

	/* Convert the character choices to internal integer codes. */
	for (n = 0, i = 1; n < nb_choice; n++, i += j) {
		j = UTF8_LENGTH(choice[i]);
		ichoice[n] = utf8_decode(choice + i) + (j > 1 ? KEY_MAX : 0);
	}

	for (;;) {
		ch = keys_wgetch(win[KEY].p);
		for (i = 0; i < nb_choice; i++)
			if (ch == ichoice[i])
				return i + 1;
		if (ch == ESCAPE)
			return (-1);
		if (ch == RETURN)
			return (-2);
		if (resize) {
			resize = 0;
			wins_reset();
			status_mesg(message, avail_choice);
		}
	}
}

/*
 * Prompts the user with a boolean question.
 *
 * Returns 1 if yes, 2 if no, and -1 otherwise
 */
int status_ask_bool(const char *msg)
{
	return (status_ask_choice(msg, _("[yn]"), 2));
}

/*
 * Prompts the user to make a choice between a number of alternatives.
 *
 * Returns the option chosen by the user (starting from 1), or -1 if
 * the user doesn't want to answer.
 */
int
status_ask_simplechoice(const char *prefix, const char *choice[],
			int nb_choice)
{
	int i;
	char *tmp;
	/* "(1) Choice1, (2) Choice2, (3) Choice3?" */
	char choicestr[BUFSIZ];
	/* Holds the characters to choose from ('1', '2', etc) */
	char char_choice[nb_choice + 2];

	/* No need to initialize first and last char. */
	for (i = 1; i <= nb_choice; i++)
		char_choice[i] = '0' + i;

	strcpy(choicestr, prefix);

	for (i = 0; i < nb_choice; i++) {
		asprintf(&tmp,
			 ((i + 1) == nb_choice) ? "(%d) %s?" : "(%d) %s, ",
			 (i + 1), choice[i]);
		strcat(choicestr, tmp);
		mem_free(tmp);
	}

	return (status_ask_choice(choicestr, char_choice, nb_choice));
}

/* Erase part of a window. */
void
erase_window_part(WINDOW * win, int first_col, int first_row, int last_col,
		  int last_row)
{
	int c, r;

	for (r = first_row; r <= last_row; r++) {
		for (c = first_col; c <= last_col; c++)
			mvwaddstr(win, r, c, " ");
	}
}

/* draws a popup window */
WINDOW *popup(int pop_row, int pop_col, int pop_y, int pop_x,
	      const char *title, const char *msg, int hint)
{
	const char *any_key = _("Press any key to continue...");
	WINDOW *popup_win;
	const int MSGXPOS = 5;

	popup_win = newwin(pop_row, pop_col, pop_y, pop_x);
	keypad(popup_win, TRUE);
	if (msg)
		mvwaddstr(popup_win, MSGXPOS, (pop_col - strlen(msg)) / 2,
			  msg);
	custom_apply_attr(popup_win, ATTR_HIGHEST);
	box(popup_win, 0, 0);
	wins_show(popup_win, title);
	if (hint)
		mvwaddstr(popup_win, pop_row - 2,
			  pop_col - (strlen(any_key) + 1), any_key);
	custom_remove_attr(popup_win, ATTR_HIGHEST);
	wins_wrefresh(popup_win);

	return popup_win;
}

/* prints in middle of a panel */
void
print_in_middle(WINDOW * win, int starty, int startx, int width,
		const char *string)
{
	int len = strlen(string);
	int x, y;

	win = win ? win : stdscr;
	getyx(win, y, x);
	x = startx ? startx : x;
	y = starty ? starty : y;
	width = width ? width : 80;

	x += (width - len) / 2;

	custom_apply_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y, x, string);
	custom_remove_attr(win, ATTR_HIGHEST);
}

/* checks if a string is only made of digits */
int is_all_digit(const char *string)
{
	for (; *string; string++) {
		if (!isdigit((int)*string))
			return 0;
	}

	return 1;
}

/* Given an item date expressed in seconds, return its start time in seconds. */
long get_item_time(time_t date)
{
	return (long)(get_item_hour(date) * HOURINSEC +
		      get_item_min(date) * MININSEC);
}

int get_item_hour(time_t date)
{
	struct tm lt;

	localtime_r(&date, &lt);
	return lt.tm_hour;
}

int get_item_min(time_t date)
{
	struct tm lt;

	localtime_r(&date, &lt);
	return lt.tm_min;
}

struct tm date2tm(struct date day, unsigned hour, unsigned min)
{
	time_t t = now();
	struct tm start;

	localtime_r(&t, &start);

	start.tm_mon = day.mm - 1;
	start.tm_mday = day.dd;
	start.tm_year = day.yyyy - 1900;
	start.tm_hour = hour;
	start.tm_min = min;
	start.tm_sec = 0;
	start.tm_isdst = -1;

	return start;
}

time_t date2sec(struct date day, unsigned hour, unsigned min)
{
	struct tm start = date2tm(day, hour, min);
	time_t t = mktime(&start);

	EXIT_IF(t == -1, _("failure in mktime"));

	return t;
}

/* Return the (calcurse) date of a (Unix) time in seconds. */
struct date sec2date(time_t t)
{
	struct tm tm;
	struct date d;

	localtime_r(&t, &tm);
	d.dd = tm.tm_mday;
	d.mm = tm.tm_mon + 1;
	d.yyyy = tm.tm_year + 1900;
	return d;
}

time_t tzdate2sec(struct date day, unsigned hour, unsigned min, char *tznew)
{
	char *tzold;
	time_t t;

	if (!tznew)
		return date2sec(day, hour, min);

	tzold = getenv("TZ");
	if (tzold)
		tzold = mem_strdup(tzold);
	setenv("TZ", tznew, 1);
	tzset();

	t = date2sec(day, hour, min);

	if (tzold) {
		setenv("TZ", tzold, 1);
		mem_free(tzold);
	} else {
	    unsetenv("TZ");
	}
	tzset();

	return t;
}

/* Compare two calcurse dates. */
int date_cmp(struct date *d1, struct date *d2)
{
	if (d1->yyyy < d2->yyyy)
		return -1;
	if (d1->yyyy > d2->yyyy)
		return 1;
	if (d1->mm < d2->mm)
		return -1;
	if (d1->mm > d2->mm)
		return 1;
	if (d1->dd < d2->dd)
		return -1;
	if (d1->dd > d2->dd)
		return 1;
	return 0;
}

/* Compare two dates (without comparing times). */
int date_cmp_day(time_t d1, time_t d2)
{
	struct tm lt1, lt2;

	localtime_r((time_t *)&d1, &lt1);
	localtime_r((time_t *)&d2, &lt2);

	if (lt1.tm_year < lt2.tm_year)
		return -1;
	if (lt1.tm_year > lt2.tm_year)
		return 1;
	if (lt1.tm_mon < lt2.tm_mon)
		return -1;
	if (lt1.tm_mon > lt2.tm_mon)
		return 1;
	if (lt1.tm_mday < lt2.tm_mday)
		return -1;
	if (lt1.tm_mday > lt2.tm_mday)
		return 1;

	return 0;
}

/* Generic function to format date. */
void date_sec2date_fmt(time_t sec, const char *fmt, char *datef)
{
#if ENABLE_NLS
	/* TODO: Find a better way to deal with localization and strftime(). */
	char *locale_old = mem_strdup(setlocale(LC_ALL, NULL));
	setlocale(LC_ALL, "C");
#endif

	struct tm lt;
	localtime_r(&sec, &lt);
	strftime(datef, BUFSIZ, fmt, &lt);

#if ENABLE_NLS
	setlocale(LC_ALL, locale_old);
	mem_free(locale_old);
#endif
}

/* Return a string containing the date, given a date in seconds. */
char *date_sec2date_str(time_t sec, const char *datefmt)
{
	char *datestr = (char *)mem_calloc(BUFSIZ, sizeof(char));
	date_sec2date_fmt(sec, datefmt, datestr);
	return datestr;
}

/*
 * Used to change date by adding a certain amount of days or months.
 * Returns 0 on success, 1 otherwise.
 */
int date_change(struct tm *date, int delta_month, int delta_day)
{
	struct tm t;

	t = *date;
	t.tm_mon += delta_month;
	t.tm_mday += delta_day;
	t.tm_isdst = -1;
	if (mktime(&t) == -1) {
		return 1;
	} else {
		t.tm_isdst = -1;
		*date = t;
		return 0;
	}
}

/*
 * Used to change date by adding a certain amount of days or months.
 */
time_t date_sec_change(time_t date, int delta_month, int delta_day)
{
	struct tm lt;
	time_t t;

	t = date;
	localtime_r(&t, &lt);
	lt.tm_mon += delta_month;
	lt.tm_mday += delta_day;
	lt.tm_isdst = -1;
	t = mktime(&lt);
	EXIT_IF(t == -1, _("failure in mktime"));

	return t;
}

/*
 * A date in seconds is updated with new day, month and year and returned.
 */
static time_t update_date_in_date(time_t date, int day, int month, int year)
{
	struct tm lt;

	localtime_r(&date, &lt);
	lt.tm_mday = day;
	lt.tm_mon = month - 1;
	lt.tm_year = year - 1900;
	lt.tm_isdst = -1;
	date = mktime(&lt);
	EXIT_IF(date == -1, _("error in mktime"));

	return date;
}

/*
 * A date in seconds is updated with new hour and minutes and returned.
 */
time_t update_time_in_date(time_t date, unsigned hr, unsigned mn)
{
	struct tm lt;

	localtime_r(&date, &lt);
	lt.tm_hour = hr;
	lt.tm_min = mn;
	lt.tm_sec = 0;
	lt.tm_isdst = -1;
	date = mktime(&lt);
	EXIT_IF(date == -1, _("error in mktime"));

	return date;
}

/*
 * Returns the date in seconds from year 1970.
 * If no date is entered, current date is chosen.
 */
time_t get_sec_date(struct date date)
{
	struct tm ptrtime;
	time_t timer;
	char current_day[] = "dd ";
	char current_month[] = "mm ";
	char current_year[] = "yyyy ";

	if (date.yyyy == 0 && date.mm == 0 && date.dd == 0) {
		timer = time(NULL);
		localtime_r(&timer, &ptrtime);
		strftime(current_day, strlen(current_day), "%d", &ptrtime);
		strftime(current_month, strlen(current_month), "%m", &ptrtime);
		strftime(current_year, strlen(current_year), "%Y", &ptrtime);
		date.mm = atoi(current_month);
		date.dd = atoi(current_day);
		date.yyyy = atoi(current_year);
	}

	return date2sec(date, 0, 0);
}

long min2sec(unsigned minutes)
{
	return minutes * MININSEC;
}

int modify_wday(int wday, int shift)
{
	return (WEEKINDAYS + wday + shift) % WEEKINDAYS;
}

/* returns char* representing a wday, used for internal functions */
char *get_wday_default_string(int wday)
{
	switch(wday) {
		case MONDAY:
			return "Monday";
			break;
		case TUESDAY:
			return "Tuesday";
			break;
		case WEDNESDAY:
			return "Wednesday";
			break;
		case THURSDAY:
			return "Thursday";
			break;
		case FRIDAY:
			return "Friday";
			break;
		case SATURDAY:
			return "Saturday";
			break;
		case SUNDAY:
			return "Sunday";
			break;
		default:
			return "Sunday";
			break;
	}
}

/*
 * Display a scroll bar when there are so many items that they
 * can not be displayed inside the corresponding panel.
 * Leave it out in the appointments panel in when multiple days mode.
 */
void draw_scrollbar(struct scrollwin *sw, int hilt)
{
	if (sw == &lb_apt.sw && conf.multiple_days)
		return;

	int y = (conf.compact_panels ? 1 : 3);
	int h = sw->h - (conf.compact_panels ? 2 : 4);

	int sbar_h = MAX(h * h / sw->line_num, 1);
	int sbar_y = y + sw->line_off * (h - sbar_h) / (sw->line_num - h);
	int sbar_x = sw->w - 1;

	/* Redraw part of the border. */
	if (hilt)
		custom_apply_attr(sw->win, ATTR_HIGHEST);
	mvwvline(sw->win, y, sbar_x, ACS_VLINE, h);
	if (hilt)
		custom_remove_attr(sw->win, ATTR_HIGHEST);

	/* Draw the scrollbar. */
	if (hilt)
		custom_apply_attr(sw->win, ATTR_HIGHEST);
	wattron(sw->win, A_REVERSE);
	mvwvline(sw->win, sbar_y, sbar_x, ' ', sbar_h);
	wattroff(sw->win, A_REVERSE);
	if (hilt)
		custom_remove_attr(sw->win, ATTR_HIGHEST);
}

/*
 * Print an item (either an appointment, event, or todo) in a
 * popup window. This is useful if an item description is too
 * long to fit in its corresponding panel window.
 */
void
item_in_popup(const char *a_start, const char *a_end, const char *msg,
	      const char *pop_title)
{
	WINDOW *popup_win, *pad;
	const int margin_left = 4, margin_top = 4;
	const int winl = row - 5, winw = col - margin_left;
	const int padl = winl - 2, padw = winw - margin_left;

	pad = newpad(padl, padw);
	popup_win = popup(winl, winw, 1, 2, pop_title, NULL, 1);
	if (a_start && a_end) {
		mvwprintw(popup_win, margin_top, margin_left, "- %s -> %s",
			  a_start, a_end);
	}
	mvwaddstr(pad, 0, margin_left, msg);
	wmove(win[STA].p, 0, 0);
	pnoutrefresh(pad, 0, 0, margin_top + 2, margin_left, padl, winw);
	wins_doupdate();
	keys_wait_for_any_key(popup_win);
	delwin(pad);
	delwin(popup_win);
}

/* Returns the beginning of current day in seconds from 1970. */
time_t get_today(void)
{
	struct tm lt;
	time_t current_time;
	struct date day;

	current_time = time(NULL);
	localtime_r(&current_time, &lt);
	day.mm = lt.tm_mon + 1;
	day.dd = lt.tm_mday;
	day.yyyy = lt.tm_year + 1900;

	return date2sec(day, 0, 0);
}

/* Returns the beginning of the selected day in the calendar. */
time_t get_slctd_day(void)
{
	return date2sec(*ui_calendar_get_slctd_day(), 0, 0);
}

/* Returns the current time in seconds. */
time_t now(void)
{
	return time(NULL);
}

char *nowstr(void)
{
	struct tm lt;
	static char buf[BUFSIZ];
	time_t t = now();

	localtime_r(&t, &lt);
	strftime(buf, sizeof buf, "%a %b %d %T %Y", &lt);

	return buf;
}

/* Print the given option value with appropriate color. */
void
print_bool_option_incolor(WINDOW * win, unsigned option, int pos_y,
			  int pos_x)
{
	int color = 0;
	const char *option_value;

	if (option == 1) {
		color = ATTR_TRUE;
		option_value = _("yes");
	} else if (option == 0) {
		color = ATTR_FALSE;
		option_value = _("no");
	} else {
		EXIT(_("option not defined"));
	}

	/*
	 * Possibly nested custom_apply_attr() calls. Turn
	 * custom_apply_attr(ATTR_HIGHEST) off explicitly,
	 * while it may have other attributes besides the colour.
	 */
	custom_remove_attr(win, ATTR_HIGHEST);
	custom_apply_attr(win, color);
	mvwaddstr(win, pos_y, pos_x, option_value);
	custom_remove_attr(win, color);
	wnoutrefresh(win);
	wins_doupdate();
}

/*
 * Get the name of the default directory for temporary files.
 */
const char *get_tempdir(void)
{
	if (getenv("TMPDIR"))
		return getenv("TMPDIR");
#ifdef P_tmpdir
	else if (P_tmpdir)
		return P_tmpdir;
#endif
	else
		return "/tmp";
}

/*
 * Create a new unique file, and return a newly allocated string which contains
 * the random part of the file name.
 */
char *new_tempfile(const char *prefix)
{
	char *fullname;
	int fd;
	FILE *file;

	if (prefix == NULL)
		return NULL;

	asprintf(&fullname, "%s.XXXXXX.md", prefix);
	if ((fd = mkstemps(fullname, 3)) == -1
	    || (file = fdopen(fd, "w+")) == NULL) {
		if (fd != -1) {
			unlink(fullname);
			close(fd);
		}
		ERROR_MSG(_("temporary file \"%s\" could not be created"),
			  fullname);

		mem_free(fullname);
		return NULL;
	}
	fclose(file);

	return fullname;
}

static void get_ymd(int *year, int *month, int *day, time_t t)
{
	struct tm tm;

	localtime_r(&t, &tm);
	*day = tm.tm_mday;
	*month = tm.tm_mon + 1;
	*year = tm.tm_year + 1900;
}

static void get_weekday_ymd(int *year, int *month, int *day, int weekday)
{
	time_t t = get_today();
	struct tm tm;
	int delta;

	localtime_r(&t, &tm);
	delta = weekday - tm.tm_wday;
	t = date_sec_change(t, 0, delta > 0 ? delta : 7);

	localtime_r(&t, &tm);
	*day = tm.tm_mday;
	*month = tm.tm_mon + 1;
	*year = tm.tm_year + 1900;
}

/*
 * Check if a calcurse date is valid.
 */
int check_date(unsigned year, unsigned month, unsigned day)
{
	return ((YEAR1902_2037 ? year >= 1902 && year <= 2037 : 1) &&
		month >= 1 && month <= 12 &&
		day >= 1 &&
		day <= days[month - 1] + (month == 2 && ISLEAP(year)) ? 1 : 0);
}

/*
 * Check that a time in seconds is a valid calcurse date (ignoring hour:min:sec).
 */
int check_sec(time_t *time)
{
	struct tm tm;
	localtime_r(time, &tm);
	return check_date(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

/*
 * Convert a string containing a date into three integers containing the year,
 * month and day.
 *
 * If a pointer to a date structure containing the current date is passed as
 * last parameter ("slctd_date"), the function will accept several short forms,
 * e.g. "26" for the 26th of the current month/year or "3/1" for Mar 01 (or Jan
 * 03, depending on the date format) of the current year. If a null pointer is
 * passed, short forms won't be accepted at all.
 *
 * Returns 1 if sucessfully converted or 0 if the string is an invalid date.
 */
int
parse_date(const char *date_string, enum datefmt datefmt, int *year,
	   int *month, int *day, struct date *slctd_date)
{
	const char sep = (datefmt == DATEFMT_ISO) ? '-' : '/';
	const char *p;
	int in[3] = { 0, 0, 0 }, n = 0;
	int d, m, y;

	if (!date_string)
		return 0;

	if (!strcasecmp(date_string, "today")) {
		get_ymd(year, month, day, get_today());
		return 1;
	} else if (!strcasecmp(date_string, "yesterday")) {
		get_ymd(year, month, day, date_sec_change(get_today(), 0, -1));
		return 1;
	} else if (!strcasecmp(date_string, "tomorrow")) {
		get_ymd(year, month, day, date_sec_change(get_today(), 0, 1));
		return 1;
	} else if (!strcasecmp(date_string, "now")) {
		get_ymd(year, month, day, now());
		return 1;
	} else if (!strcasecmp(date_string, "sunday") ||
		   !strcasecmp(date_string, "sun")) {
		get_weekday_ymd(year, month, day, 0);
		return 1;
	} else if (!strcasecmp(date_string, "monday") ||
		   !strcasecmp(date_string, "mon")) {
		get_weekday_ymd(year, month, day, 1);
		return 1;
	} else if (!strcasecmp(date_string, "tuesday") ||
		   !strcasecmp(date_string, "tue")) {
		get_weekday_ymd(year, month, day, 2);
		return 1;
	} else if (!strcasecmp(date_string, "wednesday") ||
		   !strcasecmp(date_string, "wed")) {
		get_weekday_ymd(year, month, day, 3);
		return 1;
	} else if (!strcasecmp(date_string, "thursday") ||
		   !strcasecmp(date_string, "thu")) {
		get_weekday_ymd(year, month, day, 4);
		return 1;
	} else if (!strcasecmp(date_string, "friday") ||
		   !strcasecmp(date_string, "fri")) {
		get_weekday_ymd(year, month, day, 5);
		return 1;
	} else if (!strcasecmp(date_string, "saturday") ||
		   !strcasecmp(date_string, "sat")) {
		get_weekday_ymd(year, month, day, 6);
		return 1;
	}

	/* parse string into in[], read up to three integers */
	for (p = date_string; *p; p++) {
		if (*p == sep) {
			if ((++n) > 2)
				return 0;
		} else if ((*p >= '0') && (*p <= '9')) {
			in[n] = in[n] * 10 + (int)(*p - '0');
		} else {
			return 0;
		}
	}

	if ((!slctd_date && n < 2) || in[n] == 0)
		return 0;

	/* convert into day, month and year, depending on the date format */
	switch (datefmt) {
	case DATEFMT_MMDDYYYY:
		m = (n >= 1) ? in[0] : 0;
		d = (n >= 1) ? in[1] : in[0];
		y = in[2];
		break;
	case DATEFMT_DDMMYYYY:
		d = in[0];
		m = in[1];
		y = in[2];
		break;
	case DATEFMT_YYYYMMDD:
	case DATEFMT_ISO:
		y = (n >= 2) ? in[n - 2] : 0;
		m = (n >= 1) ? in[n - 1] : 0;
		d = in[n];
		break;
	default:
		return 0;
	}

	if (slctd_date) {
		if (y > 0 && y < 100) {
			/* convert "YY" format into "YYYY" */
			y += slctd_date->yyyy - slctd_date->yyyy % 100;
		} else if (n < 2) {
			/* set year and, optionally, month if short from is used */
			y = slctd_date->yyyy;
			if (n < 1)
				m = slctd_date->mm;
		}
	}

	/* check if date is valid, take leap years into account */
	if (!check_date(y, m, d))
		return 0;

	if (year)
		*year = y;
	if (month)
		*month = m;
	if (day)
		*day = d;

	return 1;
}

int
parse_date_interactive(const char *datestr, int *year, int *month, int *day)
{
	return parse_date(datestr, conf.input_datefmt, year, month, day,
			  ui_calendar_get_slctd_day());
}

/*
 * Convert a date increment string into a number of days.
 * If start is non-zero, the final end time is validated.
 *
 * Allowed formats in lenient BNF:
 * <increment>::= <days> | <period>
 * <period>   ::= [ <weeks>w ][ <days>d ]
 * Notes:
 *            <days> and <weeks> are any integer >= 0.
 *            <period> must have at least one non-terminal.
 *
 * Returns 1 on success and 0 on failure.
 */
int parse_date_increment(const char *string, unsigned *days, time_t start)
{
	enum {
		STATE_INITIAL,
		STATE_WWDD_DD,
		STATE_DONE
	} state = STATE_INITIAL;

	const char *p;
	unsigned in = 0, frac = 0, denom = 1;
	unsigned incr = 0;

	if (!string || *string == '\0')
		return 0;

	/* parse string using a simple state machine */
	for (p = string; *p; p++) {
		if (state == STATE_DONE) {
			return 0;
		} else if ((*p >= '0') && (*p <= '9')) {
			in = in * 10 + (int)(*p - '0');
			if (frac)
				denom *= 10;
		} else if (*p == '.') {
			if (frac)
				return 0;
			frac++;
		} else {
			switch (state) {
			case STATE_INITIAL:
				if (*p == 'w') {
					incr += in * WEEKINDAYS / denom;
					state = STATE_WWDD_DD;
				} else if (*p == 'd') {
					incr += in / denom;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			case STATE_WWDD_DD:
				if (*p == 'd') {
					incr += in / denom;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			default:
				break;
			}

			in = frac = 0;
			denom = 1;
		}
	}
	if (state == STATE_DONE && in > 0)
		return 0;
	incr += in;
	if (start) {
		/* wanted: start = start + incr * DAYINSEC */
		long p;
		if (overflow_mul(incr, DAYINSEC, &p))
			return 0;
		if (overflow_add(start, p, &start))
			return 0;
		if (!check_sec(&start))
			return 0;
	}
	*days = incr;
	return 1;
}

/*
 * Check if time is valid.
 */
int check_time(unsigned hours, unsigned minutes)
{
	return (hours < DAYINHOURS && minutes < HOURINMIN);
}

/*
 * Converts a time string into hours and minutes. Short forms like "23:"
 * (23:00) or ":45" (0:45) are allowed as well as "2345". Note: the latter
 * clashes with date formats 0001 .. 0031 and must be picked up before
 * dates when parsing in parse_datetime.
 *
 * Returns 1 on success and 0 on failure.
 */
int parse_time(const char *string, unsigned *hour, unsigned *minute)
{
	const char *p;
	unsigned in[2] = { 0, 0 }, n = 0;

	if (!string)
		return 0;

	/* parse string into in[], read up to two integers */
	for (p = string; *p; p++) {
		if (*p == ':') {
			if ((++n) > 1)
				return 0;
		} else if (isdigit(*p)) {
			in[n] = in[n] * 10 + *p - '0';
		} else {
			return 0;
		}
	}
	/* 24-hour format without ':' (hhmm)? */
	if (n == 0 && strlen(string) == 4) {
		in[1] = in[0] % 100;
		in[0] = in[0] / 100;
		n = 1;
	}

	if (n != 1 || !check_time(in[0], in[1]))
		return 0;

	*hour = in[0];
	*minute = in[1];
	return 1;
}

/*
 * Converts a duration string into minutes.
 * If start time is non-zero, the final end time is validated.
 *
 * Allowed formats in lenient BNF:
 * <duration> ::= <minutes> | <time> | <period>
 * <time>     ::= <hours>:<min>
 * <period>   ::= [ <days>d ][ <hours>h ][ <minutes>m ]
 * <min>      ::= integer in the range 0-59
 * Notes:
 *            <days>, <hours> and <minutes> are any sequence of
 *	      the characters 1,2,..,9,0.
 *            <period> must have at least one non-terminal.
 *
 * Returns 1 on success and 0 on failure.
 */
int parse_duration(const char *string, unsigned *duration, time_t start)
{
	enum {
		STATE_INITIAL,
		STATE_HHMM_MM,
		STATE_DDHHMM_HH,
		STATE_DDHHMM_MM,
		STATE_DONE
	} state = STATE_INITIAL;

	const char *p;
	unsigned in = 0, frac = 0, denom = 1;
	long dur = 0;

	if (!string || *string == '\0')
		return 0;

	/* parse string using a simple state machine */
	for (p = string; *p; p++) {
		if (state == STATE_DONE) {
			return 0;
		} else if ((*p >= '0') && (*p <= '9')) {
			in = in * 10 + (int)(*p - '0');
			if (frac)
				denom *= 10;
		} else if (*p == '.') {
			if (frac)
				return 0;
			frac++;
		} else {
			switch (state) {
			case STATE_INITIAL:
				if (*p == ':') {
					dur += in * HOURINMIN / denom;
					state = STATE_HHMM_MM;
				} else if (*p == 'd') {
					dur += in * DAYINMIN / denom;
					state = STATE_DDHHMM_HH;
				} else if (*p == 'h') {
					dur += in * HOURINMIN / denom;
					state = STATE_DDHHMM_MM;
				} else if (*p == 'm') {
					dur += in / denom;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			case STATE_DDHHMM_HH:
				if (*p == 'h') {
					dur += in * HOURINMIN / denom;
					state = STATE_DDHHMM_MM;
				} else if (*p == 'm') {
					dur += in / denom;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			case STATE_DDHHMM_MM:
				if (*p == 'm') {
					dur += in / denom;
					state = STATE_DONE;
				} else {
					return 0;
				}
				break;
			case STATE_HHMM_MM:
				return 0;
			default:
				break;
			}

			in = frac = 0;
			denom = 1;
		}
	}
	if ((state == STATE_HHMM_MM && in >= HOURINMIN) ||
	    ((state == STATE_DDHHMM_HH || state == STATE_DDHHMM_MM) && in > 0))
		return 0;
	dur += in;
	if (start) {
		/* wanted: end = start + dur * MININSEC */
		time_t end;
		long p, s;
		if (overflow_mul(dur, MININSEC, &p))
			return 0;
		if (overflow_add(start, p, &s))
			return 0;
		end = s;
		if (!check_sec(&end) || end < start)
			return 0;
	}
	*duration = dur;
	return 1;
}

/*
 * Converts a string containing a date or a time into a time stamp.
 *
 * Takes a date/time string and a time stamp. If the string only contains a
 * date, the date of the time stamp is updated while the time remains
 * untouched. If the string only contains a time, the time of the time stamp is
 * updated and the date remains the same. If the string contains both a date
 * and a time, the time stamp is updated to match the given string.
 *
 * The final time is validated. In addition, if a positive duration is given,
 * time + duration validated (zero duration needs no validation).
 *
 * Returns a positive value on success and 0 on failure. The least-significant
 * bit is set if the date was updated. Bit 1 is set if the time was updated.
 */
int parse_datetime(const char *string, time_t *ts, time_t dur)
{
	unsigned hour, minute;
	int year, month, day;
	int ret = 0;

	char *d = mem_strdup(string);
	/* Split into date and time, if possible. */
	char *t = strchr(d, ' ');
	if (t)
		*t++ = '\0';
	if (t) {
		if (parse_date_interactive(d, &year, &month, &day) &&
	            parse_time(t, &hour, &minute)) {
			ret |= PARSE_DATETIME_HAS_DATE |
			       PARSE_DATETIME_HAS_TIME;
		}
	/* Time before date, see comments in parse_time(). */
	} else if (parse_time(d, &hour, &minute)) {
		ret |= PARSE_DATETIME_HAS_TIME;
	} else if (parse_date_interactive(d, &year, &month, &day)) {
		ret |= PARSE_DATETIME_HAS_DATE;
	}
	if (ret & PARSE_DATETIME_HAS_DATE)
		*ts = update_date_in_date(*ts, day, month, year);
	if (ret & PARSE_DATETIME_HAS_TIME)
		*ts = update_time_in_date(*ts, hour, minute);
	mem_free(d);

	/* Is the resulting time a valid (start or end) time? */
	if (!check_sec(ts))
		return 0;
	/* Is the resulting time + dur a valid end time? */
	if (dur) {
		/* want: sec = *ts + dur */
		time_t sec;
		if (overflow_add(*ts, dur, &sec))
			return 0;
		if (!check_sec(&sec))
			return 0;
	}
	return ret;
}

void file_close(FILE * f, const char *pos)
{
	EXIT_IF((fclose(f)) != 0, _("Error when closing file at %s"), pos);
}

/*
 * Sleep the given number of seconds, but make it more 'precise' than sleep(3)
 * (hence the 'p') in a way that even if a signal is caught during the sleep
 * process, this function will return to sleep afterwards.
 */
void psleep(unsigned secs)
{
	unsigned unslept;

	for (unslept = sleep(secs); unslept; unslept = sleep(unslept)) ;
}

/*
 * Fork and execute an external process.
 *
 * If pfdin/pfdout/pfderr point to a valid address, a pipe is created and the
 * appropriate file descriptors are written to pfdin/pfdout/pfderr.
 *
 * If new_session is non-zero, setsid() is called after forking.
 */
int fork_exec(int *pfdin, int *pfdout, int *pfderr, int new_session,
              const char *path, const char *const *arg)
{
	int pin[2], pout[2], perr[2];
	int pid;

	if (pfdin && (pipe(pin) == -1))
		return 0;
	if (pfdout && (pipe(pout) == -1))
		return 0;
	if (pfderr && (pipe(perr) == -1))
		return 0;

	if ((pid = fork()) == 0) {
		if (pfderr) {
			if (dup2(perr[0], STDERR_FILENO) < 0)
				_exit(127);
			close(perr[0]);
			close(perr[1]);
		}

		if (pfdout) {
			if (dup2(pout[0], STDIN_FILENO) < 0)
				_exit(127);
			close(pout[0]);
			close(pout[1]);
		}

		if (pfdin) {
			if (dup2(pin[1], STDOUT_FILENO) < 0)
				_exit(127);
			close(pin[0]);
			close(pin[1]);
		}

		if (new_session) {
			if ((setsid() < 0))
				_exit(127);
		}

		execvp(path, (char *const *)arg);
		_exit(127);
	} else {
		if (pfdin)
			close(pin[1]);
		if (pfdout)
			close(pout[0]);
		if (pfderr)
			close(perr[0]);

		if (pid > 0) {
			if (pfdin) {
				fcntl(pin[0], F_SETFD, FD_CLOEXEC);
				*pfdin = pin[0];
			}
			if (pfdout) {
				fcntl(pout[1], F_SETFD, FD_CLOEXEC);
				*pfdout = pout[1];
			}
			if (pfderr) {
				fcntl(perr[1], F_SETFD, FD_CLOEXEC);
				*pfderr = perr[1];
			}
		} else {
			if (pfdin)
				close(pin[0]);
			if (pfdout)
				close(pout[1]);
			if (pfderr)
				close(perr[1]);
			return 0;
		}
	}
	return pid;
}

/* Execute an external program in a shell. */
int
shell_exec(int *pfdin, int *pfdout, int *pfderr, int new_session,
           const char *path, const char *const *arg)
{
	int argc, i;
	const char **narg;
	char *arg0 = NULL;
	int ret;

	for (argc = 0; arg[argc]; argc++) ;

	if (argc < 1)
		return -1;

	narg = mem_calloc(argc + 4, sizeof(const char *));

	narg[0] = "sh";
	narg[1] = "-c";

	if (argc > 1) {
		asprintf(&arg0, "%s \"$@\"", path);
		narg[2] = arg0;

		for (i = 0; i < argc; i++)
			narg[i + 3] = arg[i];
		narg[argc + 3] = NULL;
	} else {
		narg[2] = path;
		narg[3] = NULL;
	}

	ret = fork_exec(pfdin, pfdout, pfderr, new_session, *narg, narg);

	if (arg0)
		mem_free(arg0);
	mem_free(narg);

	return ret;
}

/* Wait for a child process to terminate. */
int child_wait(int *pfdin, int *pfdout, int *pfderr, int pid)
{
	int stat;

	if (pfdin)
		close(*pfdin);
	if (pfdout)
		close(*pfdout);
	if (pfderr)
		close(*pfderr);

	if (waitpid(pid, &stat, 0) == pid)
		return stat;
	else
		return -1;
}

/* Display "Press any key to continue..." and wait for a key press. */
void press_any_key(void)
{
	struct termios t_attr_old, t_attr;

	tcgetattr(STDIN_FILENO, &t_attr_old);
	memcpy(&t_attr, &t_attr_old, sizeof(struct termios));
	t_attr.c_lflag &= ~(ICANON | ECHO | ECHONL);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_attr);

	fflush(stdout);
	fputs(_("Press any key to continue..."), stdout);
	fflush(stdout);
	fgetc(stdin);
	fflush(stdin);
	fputs("\r\n", stdout);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_attr_old);
}

/*
 * Display note contents if one is asociated with the currently displayed item
 * (to be used together with the '-a' or '-t' flag in non-interactive mode).
 * Each line begins with nbtab tabs.
 * Print "No note file found", if the notefile does not exists.
 *
 * (patch submitted by Erik Saule).
 */
static void print_notefile(FILE * out, const char *filename, int nbtab)
{
	char *path_to_notefile;
	FILE *notefile;
	char linestarter[BUFSIZ];
	char buffer[BUFSIZ];
	int i;
	int printlinestarter = 1;

	if (nbtab < BUFSIZ) {
		for (i = 0; i < nbtab; i++)
			linestarter[i] = '\t';
		linestarter[nbtab] = '\0';
	} else {
		linestarter[0] = '\0';
	}

	asprintf(&path_to_notefile, "%s/%s", path_notes, filename);
	notefile = fopen(path_to_notefile, "r");
	mem_free(path_to_notefile);
	if (notefile) {
		while (fgets(buffer, BUFSIZ, notefile) != 0) {
			if (printlinestarter) {
				fputs(linestarter, out);
				printlinestarter = 0;
			}
			fputs(buffer, out);
			if (buffer[strlen(buffer) - 1] == '\n')
				printlinestarter = 1;
		}
		fputs("\n", out);
		file_close(notefile, __FILE_POS__);
	} else {
		fputs(linestarter, out);
		fputs(_("No note file found\n"), out);
	}
}

/* Print an escape sequence and return its length. */
static int print_escape(const char *s)
{
	switch (*(s + 1)) {
	case 'a':
		putchar('\a');
		return 1;
	case 'b':
		putchar('\b');
		return 1;
	case 'f':
		putchar('\f');
		return 1;
	case 'n':
		putchar('\n');
		return 1;
	case 'r':
		putchar('\r');
		return 1;
	case 't':
		putchar('\t');
		return 1;
	case 'v':
		putchar('\v');
		return 1;
	case '0':
		putchar('\0');
		return 1;
	case '\'':
		putchar('\'');
		return 1;
	case '"':
		putchar('"');
		return 1;
	case '\?':
		putchar('?');
		return 1;
	case '\\':
		putchar('\\');
		return 1;
	case '\0':
		return 0;
	default:
		return 1;
	}
}

/* Parse a format specifier. */
static enum format_specifier parse_fs(const char **s, char *extformat)
{
	char buf[FS_EXT_MAXLEN];
	int i;

	extformat[0] = '\0';

	switch (**s) {
	case 's':
		strcpy(extformat, "epoch");
		return FS_STARTDATE;
	case 'S':
		return FS_STARTDATE;
	case 'd':
		return FS_DURATION;
	case 'e':
		strcpy(extformat, "epoch");
		return FS_ENDDATE;
	case 'E':
		return FS_ENDDATE;
	case 'm':
		return FS_MESSAGE;
	case 'n':
		return FS_NOTE;
	case 'N':
		return FS_NOTEFILE;
	case 'p':
		return FS_PRIORITY;
	case 'r':
		return FS_REMAINING;
	case '(':
		/* Long format specifier. */
		for ((*s)++, i = 0; **s != ':' && **s != ')'; (*s)++, i++) {
			if (**s == '\0')
				return FS_EOF;

			if (i < FS_EXT_MAXLEN)
				buf[i] = **s;
		}

		buf[(i < FS_EXT_MAXLEN) ? i : FS_EXT_MAXLEN - 1] = '\0';

		if (**s == ':') {
			for ((*s)++, i = 0; **s != ')'; (*s)++, i++) {
				if (**s == '\0')
					return FS_EOF;

				if (i < FS_EXT_MAXLEN)
					extformat[i] = **s;
			}

			extformat[(i <
				   FS_EXT_MAXLEN) ? i : FS_EXT_MAXLEN -
				  1] = '\0';
		}

		if (!strcmp(buf, "start"))
			return FS_STARTDATE;
		else if (!strcmp(buf, "duration"))
			return FS_DURATION;
		else if (!strcmp(buf, "end"))
			return FS_ENDDATE;
		else if (!strcmp(buf, "remaining"))
			return FS_REMAINING;
		else if (!strcmp(buf, "message"))
			return FS_MESSAGE;
		else if (!strcmp(buf, "noteid"))
			return FS_NOTE;
		else if (!strcmp(buf, "note"))
			return FS_NOTEFILE;
		else if (!strcmp(buf, "priority"))
			return FS_PRIORITY;
		else if (!strcmp(buf, "raw"))
			return FS_RAW;
		else if (!strcmp(buf, "hash"))
			return FS_HASH;
		else
			return FS_UNKNOWN;
	case '%':
		return FS_PSIGN;
	case '\0':
		return FS_EOF;
	default:
		return FS_UNKNOWN;
	}
}

/*
 * Print date to stdout, formatted to be displayed for day.
 * The "day" argument may be any time belonging to that day.
 */
static void print_date(time_t date, time_t day, const char *extformat)
{
	char buf[BUFSIZ];

	if (!strcmp(extformat, "epoch")) {
		printf("%ld", (long)date);
	} else {
		time_t day_start = DAY(day);
		time_t day_end = date_sec_change(day_start, 0, 1);
		struct tm lt;

		localtime_r((time_t *) &date, &lt);

		if (extformat[0] == '\0' || !strcmp(extformat, "default")) {
			if (date >= day_start && date <= day_end)
				strftime(buf, BUFSIZ, "%H:%M", &lt);
			else
				strftime(buf, BUFSIZ, "..:..", &lt);
		} else {
			strftime(buf, BUFSIZ, extformat, &lt);
		}

		printf("%s", buf);
	}
}

/* Print a time difference to stdout. */
static void print_datediff(long difference, const char *extformat)
{
	const char *p;
	const char *numfmt;
	bool usetotal;
	long value;

	if (!strcmp(extformat, "epoch")) {
		printf("%ld", difference);
	} else {
		if (extformat[0] == '\0' || !strcmp(extformat, "default")) {
			/* Set a default format if none specified. */
			p = "%EH:%M";
		} else {
			p = extformat;
		}
		while (*p) {
			if (*p == '%') {
				p++;
				/* Default is to zero-pad, and assume
				 * the user wants the time unit modulo
				 * the next biggest time unit. */
				numfmt = "%02d";
				usetotal = FALSE;
				if (*p == '-') {
					numfmt = "%d";
					p++;
				}
				if (*p == 'E') {
					usetotal = TRUE;
					p++;
				}
				switch (*p) {
				case '\0':
					return;
				case 'd':
					value = difference / DAYINSEC;
					printf(numfmt, value);
					break;
				case 'H':
					value = difference / HOURINSEC;
					if (!usetotal)
						value %= DAYINHOURS;
					printf(numfmt, value);
					break;
				case 'M':
					value = difference / MININSEC;
					if (!usetotal)
						value %= HOURINMIN;
					printf(numfmt, value);
					break;
				case 'S':
					value = difference;
					if (!usetotal)
						value %= MININSEC;
					printf(numfmt, value);
					break;
				case '%':
					putchar('%');
					break;
				default:
					putchar('?');
					break;
				}
			} else {
				putchar(*p);
			}
			p++;
		}
	}
}

/* Print a formatted appointment to stdout. */
static void print_apoint_helper(const char *format, time_t day,
				struct apoint *apt, struct recur_apoint *rapt)
{
	const char *p;
	char extformat[FS_EXT_MAXLEN];

	for (p = format; *p; p++) {
		if (*p == '%') {
			p++;
			switch (parse_fs(&p, extformat)) {
			case FS_STARTDATE:
				print_date(apt->start, day, extformat);
				break;
			case FS_DURATION:
				/* Backwards compatibility: Use epoch by
				 * default. */
				if (*extformat == '\0')
					strcpy(extformat, "epoch");
				print_datediff(apt->dur, extformat);
				break;
			case FS_ENDDATE:
				print_date(apt->start + apt->dur, day,
					   extformat);
				break;
			case FS_REMAINING:
				print_datediff(difftime(apt->start, now()),
				               extformat);
				break;
			case FS_MESSAGE:
				printf("%s", apt->mesg);
				break;
			case FS_NOTE:
				printf("%s", apt->note);
				break;
			case FS_NOTEFILE:
				print_notefile(stdout, apt->note, 1);
				break;
			case FS_RAW:
				if (rapt)
					recur_apoint_write(rapt, stdout);
				else
					apoint_write(apt, stdout);
				break;
			case FS_HASH:
				if (rapt)
					printf("%s", recur_apoint_hash(rapt));
				else
					printf("%s", apoint_hash(apt));
				break;
			case FS_PSIGN:
				putchar('%');
				break;
			case FS_EOF:
				return;
				break;
			default:
				putchar('?');
				break;
			}
		} else if (*p == '\\') {
			p += print_escape(p);
		} else {
			putchar(*p);
		}
	}
}

/* Print a formatted event to stdout. */
static void print_event_helper(const char *format, time_t day, struct event *ev,
			       struct recur_event *rev)
{
	const char *p;
	char extformat[FS_EXT_MAXLEN];

	for (p = format; *p; p++) {
		if (*p == '%') {
			p++;
			switch (parse_fs(&p, extformat)) {
			case FS_MESSAGE:
				printf("%s", ev->mesg);
				break;
			case FS_NOTE:
				printf("%s", ev->note);
				break;
			case FS_NOTEFILE:
				print_notefile(stdout, ev->note, 1);
				break;
			case FS_PSIGN:
				putchar('%');
				break;
			case FS_RAW:
				if (rev)
					recur_event_write(rev, stdout);
				else
					event_write(ev, stdout);
				break;
			case FS_HASH:
				if (rev)
					printf("%s", recur_event_hash(rev));
				else
					printf("%s", event_hash(ev));
				break;
			case FS_EOF:
				return;
				break;
			default:
				putchar('?');
				break;
			}
		} else if (*p == '\\') {
			p += print_escape(p);
		} else {
			putchar(*p);
		}
	}
}

/* Print a formatted appointment to stdout. */
void print_apoint(const char *format, time_t day, struct apoint *apt)
{
	print_apoint_helper(format, day, apt, NULL);
}

/* Print a formatted event to stdout. */
void print_event(const char *format, time_t day, struct event *ev)
{
	print_event_helper(format, day, ev, NULL);
}

/* Print a formatted recurrent appointment to stdout. */
void
print_recur_apoint(const char *format, time_t day, time_t occurrence,
		   struct recur_apoint *rapt)
{
	struct apoint apt;

	apt.start = occurrence;
	apt.dur = rapt->dur;
	apt.mesg = rapt->mesg;
	apt.note = rapt->note;

	print_apoint_helper(format, day, &apt, rapt);
}

/* Print a formatted recurrent event to stdout. */
void print_recur_event(const char *format, time_t day,
		       struct recur_event *rev)
{
	struct event ev;

	ev.mesg = rev->mesg;
	ev.note = rev->note;

	print_event_helper(format, day, &ev, rev);
}

/* Print a formatted todo item to stdout. */
void print_todo(const char *format, struct todo *todo)
{
	const char *p;
	char extformat[FS_EXT_MAXLEN];

	for (p = format; *p; p++) {
		if (*p == '%') {
			p++;
			switch (parse_fs(&p, extformat)) {
			case FS_PRIORITY:
				printf("%d", abs(todo->id));
				break;
			case FS_MESSAGE:
				printf("%s", todo->mesg);
				break;
			case FS_NOTE:
				printf("%s", todo->note);
				break;
			case FS_NOTEFILE:
				print_notefile(stdout, todo->note, 1);
				break;
			case FS_RAW:
				todo_write(todo, stdout);
				break;
			case FS_HASH:
				printf("%s", todo_hash(todo));
				break;
			case FS_PSIGN:
				putchar('%');
				break;
			case FS_EOF:
				return;
				break;
			default:
				putchar('?');
				break;
			}
		} else if (*p == '\\') {
			p += print_escape(p);
		} else {
			putchar(*p);
		}
	}
}

int
asprintf(char **str, const char *format, ...)
{
	struct string s;
	va_list	ap;
	int n;

	va_start(ap, format);
	string_init(&s);
	n = string_vcatf(&s, format, ap);
	*str = string_buf(&s);
	va_end(ap);

	return n;
}

int starts_with(const char *s, const char *p)
{
	for (; *p && *p == *s; s++, p++);
	return (*p == '\0');
}

int starts_with_ci(const char *s, const char *p)
{
	for (; *p && tolower(*p) == tolower(*s); s++, p++);
	return (*p == '\0');
}

int hash_matches(const char *pattern, const char *hash)
{
	int invert = 0;

	if (pattern[0] == '!') {
		invert = 1;
		pattern++;
	}

	return (starts_with(hash, pattern) != invert);
}

/*
 * Overflow check for addition with positive second term.
 */
long overflow_add(long x, long y, long *z)
{
	if (!YEAR1902_2037)
		goto exit;

	if (y < 0)
		return 1;
	if (INT_MAX - y < x)
		return 1;
   exit:
	*z = x + y;
	return 0;
}

/*
 * Overflow check for multiplication with positive terms.
 */
long overflow_mul(long x, long y, long *z)
{
	if (!YEAR1902_2037)
		goto exit;

	if (x < 0 || y <= 0)
		return 1;
	if (INT_MAX / y < x)
		return 1;
   exit:
	*z = x * y;
	return 0;
}

/*
 * Return the upcoming weekday from day (possibly day itself).
 */
time_t next_wday(time_t day, int weekday)
{
	struct tm tm;

	localtime_r(&day, &tm);
	return date_sec_change(
		day, 0, (weekday - tm.tm_wday + WEEKINDAYS) % WEEKINDAYS
	);

}

/*
 * Return the number of weekdays of the year.
 */
int wday_per_year(int year, int weekday)
{
	struct tm y_end;
	struct date day;
	int last_wday;

	/* Find weekday and yearday of the last day of the year.  */
	day.dd = 31;
	day.mm = 12;
	day.yyyy = year;
	y_end = date2tm(day, 0, 0);
	mktime(&y_end);

	/* Find date of the last weekday of the year. */
	last_wday = (y_end.tm_yday + 1) - (y_end.tm_wday - weekday + 7) % 7;

	return last_wday / 7 + (last_wday % 7 > 0);
}

/*
 * Return the number of weekdays in month of year.
 */
int wday_per_month(int month, int year, int weekday)
{
	struct tm m_end;
	struct date day;
	int last_wday, m_days = days[month - 1] + (month == 2 && ISLEAP(year) ? 1 : 0);

	/* Find weekday of the last day of the month. */
	day.dd = m_days;
	day.mm = month;
	day.yyyy = year;
	m_end = date2tm(day, 0, 0);
	mktime(&m_end);

	/* Find date of the last weekday of the month. */
	last_wday = m_days - (m_end.tm_wday - weekday + 7) % 7;

	return last_wday / 7 + (last_wday % 7 > 0);
}

/*
 * Return allocated string with day of 't' inserted in 'template' in the user's
 * preferred format; template must be a "printf" template with exactly one
 * string conversion (%s).
 */
char *day_ins(char **template, time_t t)
{
	char *day, *msg;

	day = date_sec2date_str(DAY(t), DATEFMT(conf.input_datefmt));
	asprintf(&msg, *template, day);
	mem_free(day);
	return msg;
}
