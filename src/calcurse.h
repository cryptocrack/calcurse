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

#ifndef CALCURSE_H
#define CALCURSE_H

#include "config.h"

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#elif defined HAVE_NCURSESW_NCURSES_H
#include <ncursesw/ncurses.h>
#else
#error "Missing ncurses header. Aborting..."
#endif

#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>

#include "llist.h"
#include "vector.h"
#include "htable.h"
#include "llist_ts.h"

/* Internationalization. */
#if ENABLE_NLS
#include <locale.h>
#include <libintl.h>
#undef _
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(String) (String)
#define dcgettext(String) (String)
#define ngettext(String1,String2,n) ((n) == 1 ? (String1) : (String2))
#define bindtextdomain(String) (String)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif /* ENABLE_NLS */

/* Paths configuration. */
#define DIR_NAME         "calcurse/"
#define DIR_NAME_LEGACY  ".calcurse/"
#define TODO_PATH_NAME   "todo"
#define APTS_PATH_NAME   "apts"
#define CONF_PATH_NAME   "conf"
#define KEYS_PATH_NAME   "keys"
#define CPID_PATH_NAME   ".calcurse.pid"
#define DPID_PATH_NAME   ".daemon.pid"
#define DLOG_PATH_NAME   "daemon.log"
#define NOTES_DIR_NAME   "notes/"
#define HOOKS_DIR_NAME   "hooks/"

#define DEFAULT_EDITOR     "vi"
#define DEFAULT_PAGER      "less"
#define DEFAULT_MERGETOOL  "vimdiff"

#define ATTR_FALSE    0
#define ATTR_TRUE     1
#define ATTR_LOWEST   2
#define ATTR_LOW      3
#define ATTR_MIDDLE   4
#define ATTR_HIGH     5
#define ATTR_HIGHEST  6

#define STATUSHEIGHT  2
#define MAX_NOTESIZ   40

/* Maximum number of colors available. */
#define NBUSERCOLORS  6

/* Side bar width acceptable boundaries. */
#define SBARMINWIDTH     34
#define SBARMAXWIDTHPERC 50

/* Related to date manipulation. */
#define YEARINMONTHS    12
#define YEARINDAYS      365
#define TM_YEAR_BASE    1900

#define WEEKINDAYS      7
#define DAYINHOURS      24
#define HOURINMIN       60
#define MININSEC        60

#define WEEKINHOURS     (WEEKINDAYS * DAYINHOURS)
#define WEEKINMIN       (WEEKINHOURS * HOURINMIN)
#define WEEKINSEC       (WEEKINMIN * MININSEC)
#define DAYINMIN        (DAYINHOURS * HOURINMIN)
/*
 * Note the difference between the number of seconds in a day and daylength
 * in seconds. The two may differ when DST is in effect (daylength is either
 * 23, 24 or 25 hours. The argument "date" is assumed to be of type time_t.
 */
#define DAYINSEC        (DAYINMIN * MININSEC)
#define NEXTDAY(date)	date_sec_change((date), 0, 1)
#define PREVDAY(date)	date_sec_change((date), 0, -1)
#define DAYLEN(date)	(NEXTDAY(date) - (date))
#define ENDOFDAY(date)	(NEXTDAY(date) - 1)
#define HOURINSEC       (HOURINMIN * MININSEC)
#define DAY(date)	(update_time_in_date(date, 0, 0))

/* Calendar window. */
#define CALHEIGHT       8
/*
 * Week day numbering (0, 1,..., 6) which depends on the first day of the week.
 * The argument (d) is the "Sunday"-numbering of member tm_wday in struct tm.
 */
#define WDAY(d) \
	(modify_wday(d, -ui_calendar_get_wday_start()))

/* Key definitions. */
#define CTRLVAL         0x1F
#define CTRL(x)         ((x) & CTRLVAL)
#define ESCAPE          27
#define TAB             9
#define RETURN          10
#define SPACE           32

#define KEYS_KEYLEN         3	/* length of each keybinding */
#define KEYS_LABELEN        8	/* length of command description */
#define KEYS_CMDS_PER_LINE  6	/* max number of commands per line */

/*
 * Register definitions, see ui_day.c.
 * Index 0-35 is used for '0'-'9' and 'a'-'z', see keys_get().
 */
#define REG_BLACK_HOLE 36

/* Size of the hash table the note garbage collector uses. */
#define NOTE_GC_HSIZE 1024

/* Mnemonics */
#define NOHILT		0 	/* 'No highlight' argument */
#define NOFORCE		0
#define FORCE		1
#define DUMMY		-1	/* The dummy event */

#define ERROR_MSG(...) do {                                                   \
  char msg[BUFSIZ];                                                           \
  int len;                                                                    \
                                                                              \
  len = snprintf (msg, BUFSIZ, "%s: %d: ", __FILE__, __LINE__);               \
  snprintf (msg + len, BUFSIZ - len, __VA_ARGS__);                            \
  if (ui_mode == UI_CURSES)                                                   \
    fatalbox (msg);                                                           \
  else                                                                        \
    fprintf (stderr, "%s\n", msg);                                            \
} while (0)

#define WARN_MSG(...) do {                                                    \
  char msg[BUFSIZ];                                                           \
                                                                              \
  snprintf (msg, BUFSIZ, __VA_ARGS__);                                        \
  if (ui_mode == UI_CURSES)                                                   \
    warnbox (msg);                                                            \
  else                                                                        \
    fprintf (stderr, "%s\n", msg);                                            \
} while (0)

#define EXIT(...) do {                                                        \
  ERROR_MSG(__VA_ARGS__);                                                     \
  if (ui_mode == UI_CURSES)                                                   \
    exit_calcurse (EXIT_FAILURE);                                             \
  else                                                                        \
    exit (EXIT_FAILURE);                                                      \
} while (0)

#define EXIT_IF(cond, ...) do {                                               \
  if ((cond))                                                                 \
    EXIT(__VA_ARGS__);                                                        \
} while (0)

#define RETURN_IF(cond, ...) do {                                             \
  if ((cond))                                                                 \
    {                                                                         \
      ERROR_MSG(__VA_ARGS__);                                                 \
      return;                                                                 \
    }                                                                         \
} while (0)

#define RETVAL_IF(cond, val, ...) do {                                        \
  if ((cond))                                                                 \
    {                                                                         \
      ERROR_MSG(__VA_ARGS__);                                                 \
      return (val);                                                           \
    }                                                                         \
} while (0)

#define STRINGIFY(x)  #x
#define TOSTRING(x)   STRINGIFY(x)
#define __FILE_POS__   __FILE__ ":" TOSTRING(__LINE__)

#define UTF8_MAXLEN 4
#define UTF8_LENGTH(ch) ((unsigned char)ch >= 0xF0 ? 4 : \
    ((unsigned char)ch >= 0xE0 ? 3 : \
    ((unsigned char)ch >= 0xC0 ? 2 : 1)))
#define UTF8_ISCONT(ch) ((unsigned char)ch >= 0x80 && \
    (unsigned char)ch <= 0xBF)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

enum win {
	CAL,
	APP,
	TOD,
	NOT,
	STA,
	KEY,
	NBWINS
};

enum pos {
	LEFT,
	CENTER,
	RIGHT
};

/* Available views for the calendar panel. */
enum cal_view {
	CAL_MONTH_VIEW,
	CAL_WEEK_VIEW,
	CAL_VIEWS
};

/* Available views for the todo panel. */
enum todo_view {
	TODO_SHOW_COMPLETED_VIEW,
	TODO_HIDE_COMPLETED_VIEW,
	TODO_VIEWS
};

/* General configuration variables. */
struct conf {
	enum cal_view cal_view;
	enum todo_view todo_view;
	unsigned auto_save;
	unsigned auto_gc;
	unsigned periodic_save;
	unsigned systemevents;
	unsigned confirm_quit;
	unsigned confirm_delete;
	enum win default_panel;
	unsigned compact_panels;
	unsigned multiple_days;
	unsigned header_line;
	unsigned event_separator;
	unsigned day_separator;
	unsigned empty_appt_line;
	char empty_day[BUFSIZ];
	const char *editor;
	const char *pager;
	const char *mergetool;
	char output_datefmt[BUFSIZ];	/* format for displaying date */
	int input_datefmt;	/* format for reading date */
	enum pos heading_pos; /* left/center/right for heading in appts panel */
	char day_heading[BUFSIZ];	/* format for displaying heading in appts panel */
	char timefmt[BUFSIZ];	/* format for displaying time in appts panel*/
};

#define EMPTY_DAY_DEFAULT "--"
#define EMPTY_EVENT_DESC_DEFAULT _("(empty description)")

/* Daemon-related configuration. */
struct dmon_conf {
	unsigned enable;	/* launch daemon automatically when exiting */
	unsigned log;		/* log daemon activity */
};

/* Input date formats. */
enum datefmt {
	DATEFMT_MMDDYYYY = 1,
	DATEFMT_DDMMYYYY,
	DATEFMT_YYYYMMDD,
	DATEFMT_ISO,
	DATEFMT_MAX
};

#define DATE_FORMATS (DATEFMT_MAX - 1)

#define DATEFMT(datefmt) (datefmt == DATEFMT_MMDDYYYY ? "%m/%d/%Y" :          \
    (datefmt == DATEFMT_DDMMYYYY ? "%d/%m/%Y" :                               \
    (datefmt == DATEFMT_YYYYMMDD ? "%Y/%m/%d" : "%Y-%m-%d")))

#define DATEFMT_DESC(datefmt) (datefmt == DATEFMT_MMDDYYYY ?                  \
    _("mm/dd/yyyy") : (datefmt == DATEFMT_DDMMYYYY ? _("dd/mm/yyyy") :        \
    (datefmt == DATEFMT_YYYYMMDD ? _("yyyy/mm/dd") : _("yyyy-mm-dd"))))

/* Day heading default format. */
#define DAY_HEADING_DEFAULT "%B %e, %Y"

/* Appointment time default format. */
#define APPT_TIME_DEFAULT "%H:%M"

/*
 * Calcurse representation of the date of a day in the calendar.
 * When time_t is a 32-bit signed integer, the year range is 1902 - 2037.
 */
#define YEAR1902_2037 (sizeof(time_t) == 4)
struct date {
	unsigned dd;		/* day: 1 - 31 */
	unsigned mm;		/* month: 1 - 12 */
	unsigned yyyy;		/* year AD */
};

#define ISLEAP(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

/* Appointment definition. */
struct apoint {
	time_t start;		/* seconds since 1 jan 1970 */
	long dur;		/* duration of the appointment in seconds */

#define APOINT_NULL      0x0
#define APOINT_NOTIFY    0x1	/* Item needs to be notified */
#define APOINT_NOTIFIED  0x2	/* Item was already notified */
	int state;

	char *mesg;
	char *note;
};

/* Event definition. */
struct event {
	int id;			/* event identifier */
	time_t day;		/* seconds since 1 jan 1970 */
	char *mesg;
	char *note;
};

/* Todo item definition. */
struct todo {
	char *mesg;
	int id;
	int completed;
	char *note;
};

struct excp {
	time_t st;		/* beggining of the considered day, in seconds */
};

enum recur_type {
	RECUR_DAILY,
	RECUR_WEEKLY,
	RECUR_MONTHLY,
	RECUR_YEARLY,
	NBRECUR
};

/*
 * Recurrence rule according to RFC5545; used
 * - in each recurrent appointment/event instance
 * - in passing parameters as a single function argument
 */
struct rpt {
	enum recur_type type;	/* FREQ */
	int freq;		/* INTERVAL */
	time_t until;		/* UNTIL */
	llist_t bymonth;	/* BYMONTH list */
	llist_t bywday;		/* BY(WEEK)DAY list */
	llist_t bymonthday;	/* BYMONTHDAY list */
	llist_t exc;		/* EXDATE's */
};

/* Types of integers in rrule lists. */
typedef enum {
	BYMONTH,
	BYDAY_W,
	BYDAY_M,
	BYDAY_Y,
	BYMONTHDAY,
	NOLL
} int_list_t;

/* Recurrent appointment definition. */
struct recur_apoint {
	struct rpt *rpt;	/* recurrence rule */
	llist_t exc;		/* recurrence exceptions (NOT rpt->exc) */
	time_t start;		/* start time */
	long dur;		/* duration */
	char state;		/* item state */
	char *mesg;		/* description */
	char *note;		/* attached note */
};

/* Recurrent event definition. */
struct recur_event {
	struct rpt *rpt;	/* recurrence rule */
	llist_t exc;		/* recurrence exceptions (NOT rpt->exc) */
	int id;			/* event type */
	time_t day;		/* day of the event */
	char *mesg;		/* description */
	char *note;		/* attached note */
};

/* Generic pointer data type for appointments and events. */
union aptev_ptr {
	struct apoint *apt;
	struct event *ev;
	struct recur_apoint *rapt;
	struct recur_event *rev;
};

/* Available item types in the calendar view. */
enum day_item_type {
	DAY_HEADING = 1,
	RECUR_EVNT,
	EVNT,
	EVNT_SEPARATOR,
	RECUR_APPT,
	APPT,
	EMPTY_SEPARATOR,
	END_SEPARATOR
};

/* Available item types. */
enum item_type {
	TYPE_EVNT,
	TYPE_APPT,
	TYPE_RECUR_EVNT,
	TYPE_RECUR_APPT,
	TYPE_TODO
};

/* Available item type masks. */
#define TYPE_MASK_EVNT (1 << TYPE_EVNT)
#define TYPE_MASK_APPT (1 << TYPE_APPT)
#define TYPE_MASK_RECUR_EVNT (1 << TYPE_RECUR_EVNT)
#define TYPE_MASK_RECUR_APPT (1 << TYPE_RECUR_APPT)
#define TYPE_MASK_RECUR (TYPE_MASK_RECUR_EVNT | TYPE_MASK_RECUR_APPT)
#define TYPE_MASK_CAL (TYPE_MASK_EVNT | TYPE_MASK_APPT | TYPE_MASK_RECUR)
#define TYPE_MASK_TODO (1 << TYPE_TODO)
#define TYPE_MASK_ALL (TYPE_MASK_CAL | TYPE_MASK_TODO)

/* Filter settings. */
struct item_filter {
	int invert;
	int type_mask;
	char *hash;
	regex_t *regex;
	time_t start_from;
	time_t start_to;
	time_t end_from;
	time_t end_to;
	int priority;
	int completed;
	int uncompleted;
};

/* Generic item description (to hold appointments, events...). */
struct day_item {
	enum day_item_type type;
	time_t start;
	time_t order;
	union aptev_ptr item;
};

/* Shared variables for the notification threads. */
struct notify_app {
	time_t time;
	int got_app;
	char *txt;
	char state;
	pthread_mutex_t mutex;
};

struct io_file {
	FILE *fd;
	char *name;
};

/* Virtual keys. */
enum vkey {
	KEY_GENERIC_CANCEL,
	KEY_GENERIC_SELECT,
	KEY_GENERIC_CREDITS,
	KEY_GENERIC_HELP,
	KEY_GENERIC_QUIT,
	KEY_GENERIC_SAVE,
	KEY_GENERIC_RELOAD,
	KEY_GENERIC_COPY,
	KEY_GENERIC_PASTE,
	KEY_GENERIC_CHANGE_VIEW,
	KEY_GENERIC_PREV_VIEW,
	KEY_GENERIC_IMPORT,
	KEY_GENERIC_EXPORT,
	KEY_GENERIC_GOTO,
	KEY_GENERIC_OTHER_CMD,
	KEY_GENERIC_CONFIG_MENU,
	KEY_GENERIC_REDRAW,
	KEY_GENERIC_ADD_APPT,
	KEY_GENERIC_ADD_TODO,
	KEY_GENERIC_PREV_DAY,
	KEY_GENERIC_NEXT_DAY,
	KEY_GENERIC_PREV_WEEK,
	KEY_GENERIC_NEXT_WEEK,
	KEY_GENERIC_PREV_MONTH,
	KEY_GENERIC_NEXT_MONTH,
	KEY_GENERIC_PREV_YEAR,
	KEY_GENERIC_NEXT_YEAR,
	KEY_GENERIC_SCROLL_DOWN,
	KEY_GENERIC_SCROLL_UP,
	KEY_GENERIC_GOTO_TODAY,
	KEY_GENERIC_CMD,

	KEY_MOVE_RIGHT,
	KEY_MOVE_LEFT,
	KEY_MOVE_DOWN,
	KEY_MOVE_UP,
	KEY_START_OF_WEEK,
	KEY_END_OF_WEEK,
	KEY_ADD_ITEM,
	KEY_DEL_ITEM,
	KEY_EDIT_ITEM,
	KEY_VIEW_ITEM,
	KEY_PIPE_ITEM,
	KEY_FLAG_ITEM,
	KEY_REPEAT_ITEM,
	KEY_EDIT_NOTE,
	KEY_VIEW_NOTE,
	KEY_RAISE_PRIORITY,
	KEY_LOWER_PRIORITY,

	NBVKEYS,
	KEY_UNDEF,

	/* Non-configurable, context sensitive key bindings. */
	KEY_CONFIGMENU_GENERAL,
	KEY_CONFIGMENU_LAYOUT,
	KEY_CONFIGMENU_SIDEBAR,
	KEY_CONFIGMENU_COLOR,
	KEY_CONFIGMENU_NOTIFY,
	KEY_CONFIGMENU_KEYS
};

#define FLAG_CAL (1 << CAL)
#define FLAG_APP (1 << APP)
#define FLAG_TOD (1 << TOD)
#define FLAG_NOT (1 << NOT)
#define FLAG_STA (1 << STA)
#define FLAG_ALL ((1 << NBWINS) - 1)

#define WINS_NBAR_LOCK \
  pthread_cleanup_push(wins_nbar_cleanup, NULL); \
  wins_nbar_lock();

#define WINS_NBAR_UNLOCK \
  wins_nbar_unlock(); \
  pthread_cleanup_pop(0);

#define WINS_CALENDAR_LOCK \
  pthread_cleanup_push(wins_calendar_cleanup, NULL); \
  wins_calendar_lock();

#define WINS_CALENDAR_UNLOCK \
  wins_calendar_unlock(); \
  pthread_cleanup_pop(0);

enum ui_mode {
	UI_CURSES,
	UI_CMDLINE,
	UI_MODES
};

/* Generic window structure. */
struct window {
	WINDOW *p;		/* pointer to window */
	unsigned w;		/* width */
	unsigned h;		/* height */
	int x;			/* x position */
	int y;			/* y position */
};

/* Generic scrolling window structure. */
struct scrollwin {
	WINDOW *win;		/* viewport */
	WINDOW *inner;		/* pad */
	int y;
	int x;
	int h;
	int w;
	unsigned line_off;
	unsigned line_num;
	const char *label;
};

/* Generic list box structure. */
enum listbox_row_type {
	LISTBOX_ROW_TEXT,
	LISTBOX_ROW_CAPTION
};

typedef enum listbox_row_type (*listbox_fn_item_type_t) (int, void *);
typedef int (*listbox_fn_item_height_t) (int, void *);
typedef void (*listbox_fn_draw_item_t) (int, WINDOW *, int, int, void *);

struct listbox {
	struct scrollwin sw;
	unsigned item_count;
	int item_sel;
	listbox_fn_item_type_t fn_type;
	enum listbox_row_type *type;
	listbox_fn_item_height_t fn_height;
	unsigned *ch;
	listbox_fn_draw_item_t fn_draw;
	void *cb_data;
};

/* Pad structure to handle scrolling. */
struct pad {
	int width;
	int length;
	int first_onscreen;	/* first line to be displayed inside window */
	WINDOW *ptrwin;		/* pointer to the pad window */
};

/* Notification bar definition. */
#define NOTIFY_FLAGGED_ONLY    0
#define NOTIFY_UNFLAGGED_ONLY  1
#define NOTIFY_ALL             2

struct nbar {
	unsigned show;		/* display or hide the notify-bar */
	int cntdwn;		/* warn when time left before next app
				   becomes lesser than cntdwn */
	char datefmt[BUFSIZ];	/* format for displaying date */
	char timefmt[BUFSIZ];	/* format for displaying time */
	char cmd[BUFSIZ];	/* notification command */
        const char *shell;	/* user shell to launch notif. cmd */
	unsigned notify_all;	/* notify all appointments */
	pthread_mutex_t mutex;
};

/* Dynamic strings. */
struct string {
	char *buf;
	int bufsize;
	int len;
};

/* Return codes for the getstring() function. */
enum getstr {
	GETSTRING_VALID,
	GETSTRING_ESC,		/* user pressed escape to cancel editing. */
	GETSTRING_RET		/* return was pressed without entering any text. */
};

/* Return codes for parse_datetime(). */
#define PARSE_DATETIME_HAS_DATE (1 << 0)
#define PARSE_DATETIME_HAS_TIME (1 << 1)

/* Save types and return codes for the io_save_cal() function. */
enum save_type {
	interactive,
	periodic
};
enum {
	IO_SAVE_CTINUE,
	IO_SAVE_RELOAD,
	IO_SAVE_CANCEL,
	IO_SAVE_NOOP,
	IO_SAVE_ERROR
};

/* Return codes for the io_reload_data() function. */
enum {
	IO_RELOAD_LOAD,
	IO_RELOAD_CTINUE,
	IO_RELOAD_MERGE,
	IO_RELOAD_CANCEL,
	IO_RELOAD_NOOP,
	IO_RELOAD_ERROR
};

/* Week days. */
enum wday {
	SUNDAY,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	WDAYS
};

/* Possible movements inside calendar. */
enum move {
	DAY_PREV,
	DAY_NEXT,
	WEEK_PREV,
	WEEK_NEXT,
	WEEK_START,
	WEEK_END,
	MONTH_PREV,
	MONTH_NEXT,
	YEAR_PREV,
	YEAR_NEXT
};

/*
 * Available color pairs.
 * The numbering must agree with
 * the colour numbers 1-6, see custom.c.
 */
enum {
	COLR_RED = 1,
	COLR_GREEN,
	COLR_YELLOW,
	COLR_BLUE,
	COLR_MAGENTA,
	COLR_CYAN,
	COLR_DEFAULT,
	COLR_HIGH,
	COLR_CUSTOM
};

/* Available import types. */
enum import_type {
	IO_IMPORT_ICAL,
	IO_IMPORT_NBTYPES
};

/* Available export types. */
enum export_type {
	IO_EXPORT_ICAL,
	IO_EXPORT_PCAL,
	IO_EXPORT_NBTYPES
};

/* apoint.c */
extern llist_ts_t alist_p;
void apoint_free_bkp(void);
struct apoint *apoint_dup(struct apoint *);
void apoint_free(struct apoint *);
void apoint_llist_init(void);
void apoint_llist_free(void);
struct apoint *apoint_new(char *, char *, time_t, long, char);
unsigned apoint_inday(struct apoint *, time_t *);
void apoint_sec2str(struct apoint *, time_t, char *, char *);
char *apoint_tostr(struct apoint *);
char *apoint_hash(struct apoint *);
void apoint_write(struct apoint *, FILE *);
char *apoint_scan(FILE *, struct tm, struct tm, char, char *,
			   struct item_filter *);
void apoint_delete(struct apoint *);
struct notify_app *apoint_check_next(struct notify_app *, time_t);
void apoint_switch_notify(struct apoint *);
void apoint_paste_item(struct apoint *, time_t);

/* args.c */
int parse_args(int, char **);

/* calendar.c */
extern struct day_item empty_day;

/* ui-calendar.c */
void ui_calendar_view_next(void);
void ui_calendar_view_prev(void);
void ui_calendar_set_view(int);
int ui_calendar_get_view(void);
void ui_calendar_start_date_thread(void);
void ui_calendar_stop_date_thread(void);
void ui_calendar_set_current_date(void);
struct date *ui_calendar_get_today(void);
void ui_calendar_set_first_day_of_week(enum wday);
void ui_calendar_change_first_day_of_week(void);
int ui_calendar_get_wday_start(void);
void ui_calendar_store_current_date(struct date *);
void ui_calendar_init_slctd_day(void);
struct date *ui_calendar_get_slctd_day(void);
void ui_calendar_set_slctd_day(struct date);
void ui_calendar_monthly_view_cache_set_invalid(void);
void ui_calendar_update_panel(void);
void ui_calendar_goto_today(void);
void ui_calendar_change_day(int);
void ui_calendar_move(enum move, int);
time_t ui_calendar_start_of_year(void);
time_t ui_calendar_end_of_year(void);

/* config.c */
void config_load(void);
unsigned config_save(void);

/* custom.c */
void custom_init_attr(void);
void custom_apply_attr(WINDOW *, int);
void custom_remove_attr(WINDOW *, int);
void custom_layout_config(void);
void custom_sidebar_config(void);
void custom_color_config(void);
void custom_general_config(void);
void custom_keys_config(void);
void custom_config_main(void);

/* day.c */
int day_set_sel_data(struct day_item *);
int day_check_sel_data(void);
int day_sel_index(void);
int day_get_days(void);
void day_free_vector(void);
char *day_item_get_mesg(struct day_item *);
char *day_item_get_note(struct day_item *);
void day_item_erase_note(struct day_item *);
long day_item_get_duration(struct day_item *);
int day_item_get_state(struct day_item *);
void day_item_add_exc(struct day_item *, time_t);
void day_item_fork(struct day_item *, struct day_item *);
void day_store_items(time_t, int, int);
void day_display_item_date(struct day_item *, WINDOW *, int, time_t, int, int);
void day_display_item(struct day_item *, WINDOW *, int, int, int, int);
void day_write_stdout(time_t, const char *, const char *, const char *,
		      const char *, int *);
void day_do_storage(int day_changed);
void day_popup_item(struct day_item *);
int day_check_if_item(struct date);
unsigned day_chk_busy_slices(struct date, int, int *);
struct day_item *day_cut_item(int);
int day_paste_item(struct day_item *, time_t);
struct day_item *day_get_item(int);
unsigned day_item_count(int);
void day_edit_note(struct day_item *, const char *);
void day_view_note(struct day_item *, const char *);
void day_item_switch_notify(struct day_item *);

/* dmon.c */
void dmon_start(int);
void dmon_stop(void);

/* event.c */
extern llist_t eventlist;
extern struct event dummy;
void event_free_bkp(void);
struct event *event_dup(struct event *);
void event_free(struct event *);
void event_llist_init(void);
void event_llist_free(void);
struct event *event_new(char *, char *, time_t, int);
unsigned event_inday(struct event *, time_t *);
char *event_tostr(struct event *);
char *event_hash(struct event *);
void event_write(struct event *, FILE *);
char *event_scan(FILE *, struct tm, int, char *, struct item_filter *);
void event_delete(struct event *);
void event_paste_item(struct event *, time_t);
int event_dummy(struct day_item *);

/* getstring.c */
enum getstr getstring(WINDOW *, char *, int, int, int);
int updatestring(WINDOW *, char **, int, int);

/* help.c */
int display_help(const char *);

/* hooks.c */
int run_hook(const char *);

/* ical.c */
void ical_import_data(const char *, FILE *, FILE *, unsigned *, unsigned *,
		      unsigned *, unsigned *, unsigned *, const char *,
		      const char *, const char *, const char *, const char *);
void ical_export_data(FILE *, int);

/* io.c */
unsigned io_fprintln(const char *, const char *, ...);
void io_init(const char *, const char *, const char *);
void io_extract_data(char *, const char *, int);
void io_dump_apts(const char *, const char *, const char *, const char *);
unsigned io_save_apts(const char *);
void io_dump_todo(const char *);
unsigned io_save_todo(const char *);
unsigned io_save_keys(void);
int io_save_cal(enum save_type);
void io_load_app(struct item_filter *);
void io_load_todo(struct item_filter *);
int io_load_data(struct item_filter *, int);
int io_reload_data(void);
void io_load_keys(const char *);
int io_check_dir(const char *);
unsigned io_dir_exists(const char *);
unsigned io_file_exists(const char *);
int io_check_file(const char *);
int io_check_data_files(void);
void io_export_data(enum export_type, int);
int io_import_data(enum import_type, char *, const char *, const char *,
		    const char *, const char *, const char *);
struct io_file *io_log_init(void);
void io_log_print(struct io_file *, int, const char *);
void io_log_display(struct io_file *, const char *, const char *);
void io_log_free(struct io_file *);
void io_start_psave_thread(void);
void io_stop_psave_thread(void);
void io_set_lock(void);
unsigned io_dump_pid(char *);
unsigned io_get_pid(char *);
int io_files_equal(const char *, const char *);
int io_file_cp(const char *, const char *);
void io_unset_modified(void);
void io_set_modified(void);
int io_get_modified(void);

/* keys.c */
void keys_init(void);
void keys_free(void);
void keys_dump_defaults(char *);
const char *keys_get_label(enum vkey);
const char *keys_get_binding(enum vkey);
enum vkey keys_get_action(int);
int keys_wgetch(WINDOW *);
void keys_wait_for_any_key(WINDOW *);
enum vkey keys_get(WINDOW * win, int *, int *);
int keys_assign_binding(int, enum vkey);
void keys_remove_binding(int, enum vkey);
int keys_str2int(const char *);
char *keys_int2str(int);
int keys_action_count_keys(enum vkey);
const char *keys_action_firstkey(enum vkey);
const char *keys_action_nkey(enum vkey, int);
char *keys_action_allkeys(enum vkey);
void keys_display_bindings_bar(WINDOW *, int *, int, int, int);
void keys_popup_info(enum vkey);
void keys_save_bindings(FILE *);
int keys_check_missing(void);
int keys_check_undefined(void);
int keys_fill_missing(void);

/* listbox.c */
void listbox_init(struct listbox *, int, int, int, int, const char *,
		  listbox_fn_item_type_t, listbox_fn_item_height_t,
		  listbox_fn_draw_item_t);
void listbox_delete(struct listbox *);
void listbox_resize(struct listbox *, int, int, int, int);
void listbox_set_cb_data(struct listbox *, void *);
void listbox_load_items(struct listbox *, int);
void listbox_draw_deco(struct listbox *, int);
void listbox_display(struct listbox *, int);
int listbox_get_sel(struct listbox *);
void listbox_set_sel(struct listbox *, unsigned);
void listbox_item_in_view(struct listbox *, int);
int listbox_sel_move(struct listbox *, int);

/* mem.c */
void *xmalloc(size_t);
void *xcalloc(size_t, size_t);
void *xrealloc(void *, size_t, size_t);
char *xstrdup(const char *);
void xfree(void *);

#ifdef CALCURSE_MEMORY_DEBUG

#define mem_malloc(s)         dbg_malloc ((s), __FILE_POS__)
#define mem_calloc(n, s)      dbg_calloc ((n), (s), __FILE_POS__)
#define mem_realloc(p, n, s)  dbg_realloc ((p), (n), (s), __FILE_POS__)
#define mem_strdup(s)         dbg_strdup ((s), __FILE_POS__)
#define mem_free(p)           dbg_free ((p), __FILE_POS__)

void *dbg_malloc(size_t, const char *);
void *dbg_calloc(size_t, size_t, const char *);
void *dbg_realloc(void *, size_t, size_t, const char *);
char *dbg_strdup(const char *, const char *);
void dbg_free(void *, const char *);
void mem_stats(void);

#else /* MEMORY DEBUG disabled */

#define mem_malloc(s)         xmalloc ((s))
#define mem_calloc(n, s)      xcalloc ((n), (s))
#define mem_realloc(p, n, s)  xrealloc ((p), (n), (s))
#define mem_strdup(s)         xstrdup ((s))
#define mem_free(p)           xfree ((p))
#define mem_stats()

#endif /* CALCURSE_MEMORY_DEBUG */

/* note.c */
char *generate_note(const char *);
void edit_note(char **, const char *);
void view_note(const char *, const char *);
void erase_note(char **);
void note_read(char *, FILE *);
void note_read_contents(char *, size_t, FILE *);
void note_gc(void);

/* notify.c */
int notify_time_left(void);
unsigned notify_needs_reminder(void);
void notify_update_app(time_t, char, char *);
int notify_bar(void);
void notify_init_vars(void);
void notify_init_bar(void);
void notify_free_app(void);
void notify_start_main_thread(void);
void notify_stop_main_thread(void);
void notify_reinit_bar(void);
unsigned notify_launch_cmd(void);
void notify_update_bar(void);
unsigned notify_get_next(struct notify_app *);
unsigned notify_get_next_bkgd(void);
char *notify_app_txt(void);
void notify_check_next_app(int);
void notify_check_added(char *, time_t, char);
void notify_check_repeated(struct recur_apoint *);
int notify_same_item(time_t);
int notify_same_recur_item(struct recur_apoint *);
void notify_config_bar(void);

/* pcal.c */
void pcal_export_data(FILE *);

/* recur.c */
extern llist_ts_t recur_alist_p;
extern llist_t recur_elist;
void recur_free_int_list(llist_t *);
void recur_int_list_dup(llist_t *, llist_t *);
void recur_free_exc_list(llist_t *);
void recur_exc_dup(llist_t *, llist_t *);
int recur_str2exc(llist_t *, char *);
char *recur_exc2str(llist_t *);
struct recur_event *recur_event_dup(struct recur_event *);
struct recur_apoint *recur_apoint_dup(struct recur_apoint *);
void recur_event_free_bkp(void);
void recur_apoint_free_bkp(void);
void recur_event_free(struct recur_event *);
void recur_apoint_free(struct recur_apoint *);
void recur_apoint_llist_init(void);
void recur_event_llist_init(void);
void recur_apoint_llist_free(void);
void recur_event_llist_free(void);
struct recur_apoint *recur_apoint_new(char *, char *, time_t, long, char,
				      struct rpt *);
struct recur_event *recur_event_new(char *, char *, time_t, int,
				     struct rpt *);
char recur_def2char(enum recur_type);
int recur_char2def(char);
char *recur_apoint_scan(FILE *, struct tm, struct tm, char,
				       char *, struct item_filter *,
				       struct rpt *);
char *recur_event_scan(FILE *, struct tm, int, char *,
				     struct item_filter *, struct rpt *);
char *recur_apoint_tostr(struct recur_apoint *);
char *recur_apoint_hash(struct recur_apoint *);
void recur_apoint_write(struct recur_apoint *, FILE *);
char *recur_event_tostr(struct recur_event *);
char *recur_event_hash(struct recur_event *);
void recur_event_write(struct recur_event *, FILE *);
void recur_save_data(FILE *);
unsigned recur_item_find_occurrence(time_t, long, struct rpt *, llist_t *,
				    time_t, time_t *);
unsigned recur_apoint_find_occurrence(struct recur_apoint *, time_t, time_t *);
unsigned recur_event_find_occurrence(struct recur_event *, time_t, time_t *);
unsigned recur_item_inday(time_t, long, struct rpt *, llist_t *, time_t);
unsigned recur_apoint_inday(struct recur_apoint *, time_t *);
unsigned recur_event_inday(struct recur_event *, time_t *);
void recur_event_add_exc(struct recur_event *, time_t);
void recur_apoint_add_exc(struct recur_apoint *, time_t);
void recur_event_erase(struct recur_event *);
void recur_apoint_erase(struct recur_apoint *);
void recur_bymonth(llist_t *, FILE *);
void recur_bywday(enum recur_type, llist_t *, FILE *);
void recur_bymonthday(llist_t *, FILE *);
void recur_exc_scan(llist_t *, FILE *);
void recur_apoint_check_next(struct notify_app *, time_t, time_t);
void recur_apoint_switch_notify(struct recur_apoint *);
void recur_event_paste_item(struct recur_event *, time_t);
void recur_apoint_paste_item(struct recur_apoint *, time_t);
int recur_next_occurrence(time_t, long, struct rpt *, llist_t *, time_t, time_t *);
int recur_nth_occurrence(time_t, long, struct rpt *, llist_t *, int, time_t *);
int recur_prev_occurrence(time_t, long, struct rpt *, llist_t *, time_t, time_t *);


/* sigs.c */
void sigs_init(void);
unsigned sigs_set_hdlr(int, void (*)(int));
void sigs_ignore(void);
void sigs_unignore(void);

/* strings.c */
void string_init(struct string *);
void string_reset(struct string *);
int string_grow(struct string *, int);
char *string_buf(struct string *);
int string_catf(struct string *, const char *, ...);
int string_vcatf(struct string *, const char *, va_list);
int string_printf(struct string *, const char *, ...);
int string_catftime(struct string *, const char *, const struct tm *);
int string_strftime(struct string *, const char *, const struct tm *);

/* todo.c */
extern llist_t todolist;
struct todo *todo_get_item(int, int);
struct todo *todo_add(char *, int, int, char *);
char *todo_tostr(struct todo *);
char *todo_hash(struct todo *);
void todo_write(struct todo *, FILE *);
void todo_delete_note(struct todo *);
void todo_delete(struct todo *);
void todo_resort(struct todo *);
void todo_flag(struct todo *);
int todo_get_position(struct todo *, int);
void todo_edit_note(struct todo *, const char *);
void todo_view_note(struct todo *, const char *);
void todo_free(struct todo *);
void todo_init_list(void);
void todo_free_list(void);

/* ui-day.c */
void ui_day_item_add(void);
void ui_day_item_delete(unsigned);
void ui_day_item_edit(void);
void ui_day_item_pipe(void);
void ui_day_item_repeat(void);
void ui_day_item_cut(unsigned);
void ui_day_item_cut_free(unsigned);
void ui_day_item_copy(unsigned);
void ui_day_item_paste(unsigned);
void ui_day_load_items(void);
void ui_day_find_sel(void);
struct day_item *ui_day_get_sel(void);
time_t ui_day_sel_date(void);
void ui_day_sel_reset(void);
void ui_day_set_sel(struct day_item *);
int ui_day_sel_move(int);
void ui_day_sel_daybegin(int);
void ui_day_sel_dayend(void);
void ui_day_draw(int, WINDOW *, int, int, void *);
enum listbox_row_type ui_day_row_type(int, void *);
int ui_day_height(int, void *);
void ui_day_update_panel(int);
void ui_day_popup_item(void);
void ui_day_flag(void);
void ui_day_view_note(void);
void ui_day_edit_note(void);

/* ui-todo.c */
void ui_todo_add(void);
void ui_todo_delete(void);
void ui_todo_edit(void);
void ui_todo_pipe(void);
void ui_todo_draw(int, WINDOW *, int, int, void *);
enum listbox_row_type ui_todo_row_type(int, void *);
int ui_todo_height(int, void *);
void ui_todo_load_items(void);
void ui_todo_sel_reset(void);
void ui_todo_sel_move(int);
void ui_todo_update_panel(int);
void ui_todo_chg_priority(int);
void ui_todo_popup_item(void);
void ui_todo_flag(void);
void ui_todo_view_note(void);
void ui_todo_edit_note(void);
void ui_todo_view_prev(void);
void ui_todo_view_next(void);
int ui_todo_get_view(void);
void ui_todo_set_view(int);

/* utf8.c */
int utf8_decode(const char *);
int utf8_width(char *);
int utf8_strwidth(char *);
int utf8_chop(char *, int);
char *utf8_encode(int);

/* utils.c */
void exit_calcurse(int) __attribute__ ((__noreturn__));
void free_user_data(void);
void fatalbox(const char *);
void warnbox(const char *);
void status_mesg(const char *, const char *);
int status_ask_choice(const char *, const char[], int);
int status_ask_bool(const char *);
int status_ask_simplechoice(const char *, const char *[], int);
void erase_window_part(WINDOW *, int, int, int, int);
WINDOW *popup(int, int, int, int, const char *, const char *, int);
void print_in_middle(WINDOW *, int, int, int, const char *);
int is_all_digit(const char *);
long get_item_time(time_t);
int get_item_hour(time_t);
int get_item_min(time_t);
struct tm date2tm(struct date, unsigned, unsigned);
time_t date2sec(struct date, unsigned, unsigned);
struct date sec2date(time_t);
time_t tzdate2sec(struct date, unsigned, unsigned, char *);
int date_cmp(struct date *, struct date *);
int date_cmp_day(time_t, time_t);
char *date_sec2date_str(time_t, const char *);
void date_sec2date_fmt(time_t, const char *, char *);
int date_change(struct tm *, int, int);
time_t date_sec_change(time_t, int, int);
time_t update_time_in_date(time_t, unsigned, unsigned);
time_t get_sec_date(struct date);
long min2sec(unsigned);
int modify_wday(int,int);
char *get_wday_default_string(int);
void draw_scrollbar(struct scrollwin *, int);
void item_in_popup(const char *, const char *, const char *, const char *);
time_t get_today(void);
time_t get_slctd_day(void);
time_t now(void);
char *nowstr(void);
void print_bool_option_incolor(WINDOW *, unsigned, int, int);
const char *get_tempdir(void);
char *new_tempfile(const char *);
int check_date(unsigned, unsigned, unsigned);
int parse_date(const char *, enum datefmt, int *, int *, int *, struct date *);
int parse_date_interactive(const char *, int *, int *, int *);
int check_sec(time_t *);
int check_time(unsigned, unsigned);
int parse_time(const char *, unsigned *, unsigned *);
int parse_duration(const char *, unsigned *, time_t);
int parse_date_increment(const char *, unsigned *, time_t);
int parse_datetime(const char *, time_t *, time_t);
void file_close(FILE *, const char *);
void psleep(unsigned);
int fork_exec(int *, int *, int *, int, const char *, const char *const *);
int shell_exec(int *, int *, int *, int, const char *, const char *const *);
int child_wait(int *, int *, int *, int);
void press_any_key(void);
void print_apoint(const char *, time_t, struct apoint *);
void print_event(const char *, time_t, struct event *);
void print_recur_apoint(const char *, time_t, time_t, struct recur_apoint *);
void print_recur_event(const char *, time_t, struct recur_event *);
void print_todo(const char *, struct todo *);
int vasprintf(char **, const char *, va_list);
int asprintf(char **, const char *, ...);
int starts_with(const char *, const char *);
int starts_with_ci(const char *, const char *);
int hash_matches(const char *, const char *);
long overflow_add(long, long, long *);
long overflow_mul(long, long, long *);
time_t next_wday(time_t, int);
int wday_per_year(int, int);
int wday_per_month(int, int, int);
char *day_ins(char **, time_t);

/* vars.c */
extern int col, row;
extern int resize;
extern unsigned colorize;
extern int foreground, background;
extern enum ui_mode ui_mode;
extern int read_only;
extern int quiet;
extern int want_reload;
extern const char *datefmt_str[];
extern int days[];
extern char *path_ddir;
extern char *path_cdir;
extern char *path_todo;
extern char *path_apts;
extern char *path_conf;
extern char *path_keys;
extern char *path_notes;
extern char *path_cpid;
extern char *path_dpid;
extern char *path_dmon_log;
extern char *path_hooks;
extern struct conf conf;
extern struct pad apad;
extern struct nbar nbar;
extern struct dmon_conf dmon;
void vars_init(void);
extern pthread_t notify_t_main, io_t_psave, ui_calendar_t_date;

/* wins.c */
extern struct window win[NBWINS];
extern struct scrollwin sw_cal;
extern struct listbox lb_apt;
extern struct listbox lb_todo;
unsigned wins_nbar_lock(void);
void wins_nbar_unlock(void);
void wins_nbar_cleanup(void *);
unsigned wins_calendar_lock(void);
void wins_calendar_unlock(void);
void wins_calendar_cleanup(void *);
int wins_refresh(void);
int wins_wrefresh(WINDOW *);
int wins_doupdate(void);
int wins_redrawwin(WINDOW *);
int wins_layout(void);
void wins_set_layout(int);
unsigned wins_sbar_width(void);
unsigned wins_sbar_wperc(void);
void wins_set_sbar_width(unsigned);
void wins_sbar_winc(void);
void wins_sbar_wdec(void);
enum win wins_slctd(void);
void wins_slctd_set(enum win);
void wins_slctd_next(void);
void wins_slctd_prev(void);
void wins_init(void);
void wins_scrollwin_init(struct scrollwin *, int, int, int, int, const char *);
void wins_scrollwin_resize(struct scrollwin *, int, int, int, int);
void wins_scrollwin_set_pad(struct scrollwin *, unsigned);
void wins_scrollwin_delete(struct scrollwin *);
void wins_scrollwin_draw_deco(struct scrollwin *, int);
void wins_scrollwin_display(struct scrollwin *, int);
void wins_scrollwin_up(struct scrollwin *, int);
void wins_scrollwin_down(struct scrollwin *, int);
void wins_scrollwin_in_view(struct scrollwin *, unsigned);
void wins_resize(void);
void wins_resize_panels(void);
void wins_show(WINDOW *, const char *);
void wins_get_config(void);
void wins_update_border(int);
void wins_update_panels(int);
void wins_update(int);
void wins_reset_noupdate(void);
void wins_reset(void);
void wins_prepare_external(void);
void wins_unprepare_external(void);
void wins_launch_external(const char *[]);
void wins_set_bindings(int *, int);
void wins_update_bindings(void);
void wins_status_bar(void);
void wins_erase_status_bar(void);
void wins_other_status_page();
void wins_reset_status_page(void);

/* queue.c */
void que_init(void);
int que_ued(void);
struct event *que_ins(char *, time_t, int);
void que_rem(void);
void que_show(void);
void que_save(void);
#endif /* CALCURSE_H */
