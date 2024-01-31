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

#include "calcurse.h"

/*
 * variables to store window size
 */
int col, row;
int resize;

/* variable to tell if the terminal supports color */
unsigned colorize;

/* Default background and foreground colors. */
int foreground, background;

/*
 * To tell if curses interface was launched already or not (in that case
 * calcurse is running in command-line mode).
 * This is useful to konw how to display messages on the screen.
 */
enum ui_mode ui_mode = UI_CMDLINE;

/* Don't save anything if this is set. */
int read_only;

/* Hide import/export message if set. */
int quiet;

/* Applications can trigger a reload by sending SIGUSR1. */
int want_reload;

/* Strings describing each input date format. */
const char *datefmt_str[DATE_FORMATS];

/*
 * variable to store month lengths
 */
int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
 * variables to store data path names, which are initialized in
 * io_init()
 */
char *path_ddir;
char *path_cdir;
char *path_todo;
char *path_apts;
char *path_conf;
char *path_notes;
char *path_keys;
char *path_cpid;
char *path_dpid;
char *path_dmon_log;
char *path_hooks;

/* Variable to store global configuration. */
struct conf conf;

/* Variable to handle pads. */
struct pad apad;

/* Variable to store notify-bar settings. */
struct nbar nbar;

/* Variable to store daemon configuration. */
struct dmon_conf dmon;

/*
 * Thread id variables for threads that never exit.
 *
 * Each variable either carries the identifier of the corresponding thread. If
 * one of the threads is not running, the corresponding variable is assigned
 * the identifier of the main thread instead.
 */
pthread_t notify_t_main, io_t_psave, ui_calendar_t_date;

/*
 * Variables init
 */
void vars_init(void)
{
	const char *ed, *pg, *mt;

	/* Variables for user configuration */
	conf.cal_view = CAL_MONTH_VIEW;
	conf.todo_view = TODO_HIDE_COMPLETED_VIEW;
	conf.multiple_days = 1;
	conf.header_line = 1;
	conf.event_separator = 1;
	conf.day_separator = 1;
	conf.empty_appt_line = 1;
	strcpy(conf.empty_day, EMPTY_DAY_DEFAULT);
	conf.confirm_quit = 1;
	conf.confirm_delete = 1;
	conf.auto_save = 1;
	conf.auto_gc = 0;
	conf.periodic_save = 0;
	conf.systemevents = 1;
	conf.default_panel = CAL;
	conf.compact_panels = 0;
	strncpy(conf.output_datefmt, "%D", 3);
	conf.input_datefmt = 1;
	conf.heading_pos = RIGHT;
	strcpy(conf.day_heading, DAY_HEADING_DEFAULT);
	strcpy(conf.timefmt, APPT_TIME_DEFAULT);

	datefmt_str[0] = _("mm/dd/yyyy");
	datefmt_str[1] = _("dd/mm/yyyy");
	datefmt_str[2] = _("yyyy/mm/dd");
	datefmt_str[3] = _("yyyy-mm-dd");

	/* Default external editor and pager */
	ed = getenv("CALCURSE_EDITOR");
	if (ed == NULL || ed[0] == '\0')
		ed = getenv("VISUAL");
	if (ed == NULL || ed[0] == '\0')
		ed = getenv("EDITOR");
	if (ed == NULL || ed[0] == '\0')
		ed = DEFAULT_EDITOR;
	conf.editor = ed;

	pg = getenv("CALCURSE_PAGER");
	if (pg == NULL || pg[0] == '\0')
		pg = getenv("PAGER");
	if (pg == NULL || pg[0] == '\0')
		pg = DEFAULT_PAGER;
	conf.pager = pg;

	mt = getenv("CALCURSE_MERGETOOL");
	if (mt == NULL || mt[0] == '\0')
		mt = getenv("MERGETOOL");
	if (mt == NULL || mt[0] == '\0')
		mt = DEFAULT_MERGETOOL;
	conf.mergetool = mt;

	wins_set_layout(1);

	ui_calendar_set_first_day_of_week(MONDAY);

	/* Pad structure to scroll text inside the appointment panel */
	apad.length = 1;
	apad.first_onscreen = 0;

	/* Attribute definitions for color and non-color terminals */
	custom_init_attr();

	/* Start at the current date */
	ui_calendar_init_slctd_day();

	/* Threads not yet running. */
	notify_t_main = io_t_psave = ui_calendar_t_date = pthread_self();
}
