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
#include <sys/types.h>
#include <ctype.h>
#include <time.h>

#include "calcurse.h"

static vector_t day_items;
static unsigned day_items_nb;

struct day_item empty_day;

/*
 * The day vector, day_items, is continuously rebuilt for display as the
 * selected day changes, items are added, edited or deleted etc. To keep track
 * of the selected item of the day vector across rebuilds, data are saved in
 * here that may later be used to refind the item in the rebuilt day
 * vector.
 */
static struct day_item sel_data;

/*
 * Save the item to become the selected APP item.
 * Public function used to override the setting in day_do_storage().
 */
int day_set_sel_data(struct day_item *d)
{
	if (!d)
		return 0;

	sel_data = *d;
	return 1;
}

/*
 * Return selection data if available.
 */
int day_check_sel_data()
{
	return (sel_data.order || sel_data.item.apt) ? 1 : 0;
}

/*
 * Return the position of the saved selection in the day vector.
 */
int day_sel_index(void)
{
	int i = 0;
	struct day_item *p;

	VECTOR_FOREACH(&day_items, i) {
		p = VECTOR_NTH(&day_items, i);
		if (p->order == sel_data.order &&
		    p->item.apt == sel_data.item.apt)
			return i;
	}
	/* Needed as long as ui_day_item_paste() does not set order. */
	VECTOR_FOREACH(&day_items, i) {
		p = VECTOR_NTH(&day_items, i);
		if (p->item.apt == sel_data.item.apt)
			return i;
	}
	/* If still not found, stay on the same day. */
	VECTOR_FOREACH(&day_items, i) {
		p = VECTOR_NTH(&day_items, i);
		if (p->order == DAY(sel_data.order))
			return i;
	}
	return -1;
}

/*
 * Return the number of days to load in the appointments panel.
 */
int day_get_days(void)
{
	int panel, day;

	if (!conf.multiple_days)
		return 1;

	panel = win[APP].h - (conf.compact_panels ? 2 : 4);
	/* Assume one event per day (no event separator). */
	day = 2 + conf.header_line + conf.day_separator + conf.empty_appt_line;

	if (panel <= day)
		return 2;

	/* Round up. */
	return panel / day + (panel % day ? 1 : 0);
}

static void day_free(struct day_item *day)
{
	mem_free(day);
}

static void day_init_vector(void)
{
	day_items_nb = 0;
	VECTOR_INIT(&day_items, 16);
}

/*
 * Free the current day vector containing the events and appointments.
 * Must not free associated message and note, because their are not dynamically
 * allocated (only pointers to real objects are stored in this structure).
 */
void day_free_vector(void)
{
	VECTOR_FREE_INNER(&day_items, day_free);
	VECTOR_FREE(&day_items);
}

static int day_cmp(struct day_item **pa, struct day_item **pb)
{
	struct day_item *a = *pa;
	struct day_item *b = *pb;
	int a_state, b_state;

	if (a->order < b->order)
		return -1;
	if (a->order > b->order)
		return 1;

	if ((a->type == APPT || a->type == RECUR_APPT) &&
	    (b->type == APPT || b->type == RECUR_APPT)) {
		if (a->start < b->start)
			return -1;
		if (a->start > b->start)
			return 1;

		a_state = day_item_get_state(a);
		b_state = day_item_get_state(b);
		if ((a_state & APOINT_NOTIFY) && !(b_state & APOINT_NOTIFY))
			return -1;
		if (!(a_state & APOINT_NOTIFY) && (b_state & APOINT_NOTIFY))
			return 1;

		return strcmp(day_item_get_mesg(a), day_item_get_mesg(b));
	} else if ((a->type == EVNT || a->type == RECUR_EVNT) &&
		   (b->type == EVNT || b->type == RECUR_EVNT)) {
		return strcmp(day_item_get_mesg(a), day_item_get_mesg(b));
	}

	return a->type - b->type;
}

/* Add an item to the current day list. */
static void day_add_item(int type, time_t start, time_t order, union aptev_ptr item)
{
	struct day_item *day = mem_malloc(sizeof(struct day_item));
	day->type = type;
	day->start = start;
	day->order = order;
	day->item = item;

	VECTOR_ADD(&day_items, day);
}

/* Get the message of an item. */
char *day_item_get_mesg(struct day_item *day)
{
	switch (day->type)
	{
	case APPT:
		return day->item.apt->mesg;
	case EVNT:
		return day->item.ev->mesg;
	case RECUR_APPT:
		return day->item.rapt->mesg;
	case RECUR_EVNT:
		return day->item.rev->mesg;
	default:
		return NULL;
	}
}

/* Get the display message of an item. */
char *day_item_get_display_mesg(struct day_item *day)
{
	char *msg = day_item_get_mesg(day);
	if (msg[0] == '\0')
		return EMPTY_EVENT_DESC_DEFAULT;
	return msg;
}

/* Get the note attached to an item. */
char *day_item_get_note(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		return day->item.apt->note;
	case EVNT:
		return day->item.ev->note;
	case RECUR_APPT:
		return day->item.rapt->note;
	case RECUR_EVNT:
		return day->item.rev->note;
	default:
		return NULL;
	}
}

/* Get the note attached to an item. */
void day_item_erase_note(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		erase_note(&day->item.apt->note);
		break;
	case EVNT:
		erase_note(&day->item.ev->note);
		break;
	case RECUR_APPT:
		erase_note(&day->item.rapt->note);
		break;
	case RECUR_EVNT:
		erase_note(&day->item.rev->note);
		break;
	default:
		break;
	}
}

/* Get the duration of an item. */
long day_item_get_duration(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		return day->item.apt->dur;
	case RECUR_APPT:
		return day->item.rapt->dur;
	default:
		return 0;
	}
}

/* Get the notification state of an item. */
int day_item_get_state(struct day_item *day)
{
	switch (day->type) {
	case APPT:
		return day->item.apt->state;
	case RECUR_APPT:
		return day->item.rapt->state;
	default:
		return APOINT_NULL;
	}
}

/* Add an exception to an item. */
void day_item_add_exc(struct day_item *day, time_t date)
{
	switch (day->type) {
	case RECUR_EVNT:
		recur_event_add_exc(day->item.rev, date);
		break;
	case RECUR_APPT:
		recur_apoint_add_exc(day->item.rapt, date);
		break;
	default:
		break;
	}
}

/* Clone the actual item. */
void day_item_fork(struct day_item *day_in, struct day_item *day_out)
{
	day_out->type = day_in->type;
	day_out->start = day_in->start;
	day_out->order = day_in->order;

	switch (day_in->type) {
	case APPT:
		day_out->item.apt = apoint_dup(day_in->item.apt);
		break;
	case EVNT:
		day_out->item.ev = event_dup(day_in->item.ev);
		break;
	case RECUR_APPT:
		day_out->item.rapt = recur_apoint_dup(day_in->item.rapt);
		break;
	case RECUR_EVNT:
		day_out->item.rev = recur_event_dup(day_in->item.rev);
		break;
	default:
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}
}

/*
 * Store the events for the selected day in structure pointed
 * by day_items. This is done by copying the events
 * from the general structure pointed by eventlist to the structure
 * dedicated to the selected day.
 * Returns the number of events for the selected day.
 */
static int day_store_events(time_t date)
{
	llist_item_t *i;
	union aptev_ptr p;
	int e_nb = 0;

	LLIST_FIND_FOREACH_CONT(&eventlist, &date, event_inday, i) {
		struct event *ev = LLIST_TS_GET_DATA(i);

		p.ev = ev;
		day_add_item(EVNT, ev->day, ev->day, p);
		e_nb++;
	}

	return e_nb;
}

/*
 * Store the recurrent events for the selected day in structure pointed
 * by day_items. This is done by copying the recurrent events
 * from the general structure pointed by recur_elist to the structure
 * dedicated to the selected day.
 * Returns the number of recurrent events for the selected day.
 */
static int day_store_recur_events(time_t date)
{
	llist_item_t *i;
	union aptev_ptr p;
	int e_nb = 0;

	LLIST_FIND_FOREACH(&recur_elist, &date, recur_event_inday, i) {
		struct recur_event *rev = LLIST_TS_GET_DATA(i);

		p.rev = rev;
		time_t occurrence;
		if (recur_event_find_occurrence(rev, date, &occurrence)) {
			day_add_item(RECUR_EVNT, occurrence, occurrence, p);
			e_nb++;
		}
	}

	return e_nb;
}

/*
 * Store the apoints for the selected day in structure pointed
 * by day_items. This is done by copying the appointments
 * from the general structure pointed by alist_p to the
 * structure dedicated to the selected day.
 * Returns the number of appointments for the selected day.
 */
static int day_store_apoints(time_t date)
{
	llist_item_t *i;
	union aptev_ptr p;
	int a_nb = 0;

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FIND_FOREACH(&alist_p, &date, apoint_inday, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);

		p.apt = apt;
		/*
		 * For appointments continuing from the previous day, order is
		 * set to midnight to sort it before appointments of the day.
		 */
		day_add_item(APPT,
			     apt->start,
			     apt->start < date ? date : apt->start,
			     p);
		a_nb++;
	}
	LLIST_TS_UNLOCK(&alist_p);

	return a_nb;
}

/*
 * Store the recurrent apoints for the selected day in structure pointed
 * by day_items. This is done by copying the appointments
 * from the general structure pointed by recur_alist_p to the
 * structure dedicated to the selected day.
 * Returns the number of recurrent appointments for the selected day.
 */
static int day_store_recur_apoints(time_t date)
{
	llist_item_t *i;
	union aptev_ptr p;
	time_t occurrence;
	int a_nb = 0;

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FIND_FOREACH(&recur_alist_p, &date, recur_apoint_inday, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);

		p.rapt = rapt;
		/* As for appointments */
		if (recur_apoint_find_occurrence(rapt, date, &occurrence)) {
			day_add_item(RECUR_APPT,
				     occurrence,
				     occurrence < date ? date : occurrence,
				     p);
			a_nb++;
		}
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	return a_nb;
}

/*
 * Store all of the items to be displayed for the selected day and the following
 * (n - 1) days. Items are of four types: recursive events, normal events,
 * recursive appointments and normal appointments.
 * The items are stored in the day_items vector; the number of events and
 * appointments in the vector is stored in day_items_nb,
 */
void
day_store_items(time_t date, int include_captions, int n)
{
	unsigned apts, events;
	union aptev_ptr p = { NULL }, d;
	int i;

	day_free_vector();
	day_init_vector();

	for (i = 0; i < n; i++, date = NEXTDAY(date)) {
		if (YEAR1902_2037 && !check_sec(&date))
			break;

		if (include_captions)
			day_add_item(DAY_HEADING, 0, date, p);

		events = day_store_recur_events(date);
		events += day_store_events(date);
		apts = day_store_recur_apoints(date);
		apts += day_store_apoints(date);

		if (include_captions && events > 0 && apts > 0)
			day_add_item(EVNT_SEPARATOR, 0, date, p);

		day_items_nb += events + apts;

		if (include_captions && events == 0 && apts == 0) {
			/* Insert dummy event. */
			d.ev = &dummy;
			dummy.mesg = conf.empty_day;
			day_add_item(EVNT, DUMMY, date, d);
			day_items_nb++;
		}

		if (include_captions) {
			/* Empty line at end of day if appointments have one. */
			if (apts == 0 && conf.empty_appt_line)
				day_add_item(EMPTY_SEPARATOR, 0, ENDOFDAY(date), p);
			day_add_item(END_SEPARATOR, 0, ENDOFDAY(date), p);
		}
	}

	VECTOR_SORT(&day_items, day_cmp);
}

/*
 * Print an item date in the appointment panel.
 */
void
day_display_item_date(struct day_item *day, WINDOW *win, int incolor,
		      time_t date, int y, int x)
{
	char a_st[100], a_end[100];
	char ch_recur, ch_notify;

	/* FIXME: Redesign apoint_sec2str() and remove the need for a temporary
	 * appointment item here. */
	struct apoint apt_tmp;
	apt_tmp.start = day->start;
	apt_tmp.dur = day_item_get_duration(day);

	apoint_sec2str(&apt_tmp, date, a_st, a_end);
	if (incolor == 0)
		custom_apply_attr(win, ATTR_HIGHEST);
	ch_recur = (day->type == RECUR_EVNT ||
			day->type == RECUR_APPT) ? '*' : '-';
	ch_notify = (day_item_get_state(day) & APOINT_NOTIFY) ? '!' : ' ';
	mvwprintw(win, y, x, " %c%c%s", ch_recur, ch_notify, a_st);
	if (apt_tmp.dur)
		mvwprintw(win, y, x + 3 + strlen(a_st), " -> %s", a_end);
	if (incolor == 0)
		custom_remove_attr(win, ATTR_HIGHEST);
}

/*
 * Print an item description in the corresponding panel window.
 */
void
day_display_item(struct day_item *day, WINDOW *win, int incolor, int width,
		 int y, int x)
{
	int ch_recur, ch_note;
	char buf[width * UTF8_MAXLEN];

	if (width <= 0)
		return;

	char *mesg = day_item_get_display_mesg(day);

	ch_recur = (day->type == RECUR_EVNT) ? '*' : ' ';
	ch_note = day_item_get_note(day) ? '>' : ' ';

	strncpy(buf, mesg, width * UTF8_MAXLEN);
	buf[sizeof(buf) - 1] = '\0';
	utf8_chop(buf, width - 3);

	if (!incolor)
		custom_apply_attr(win, ATTR_HIGHEST);
	mvwprintw(win, y, x, " %c%c%s", ch_recur, ch_note, buf);
	if (!incolor)
		custom_remove_attr(win, ATTR_HIGHEST);
}

/* Write the appointments and events for the selected day to stdout. */
void day_write_stdout(time_t date, const char *fmt_apt, const char *fmt_rapt,
		      const char *fmt_ev, const char *fmt_rev, int *limit)
{
	int i;

	VECTOR_FOREACH(&day_items, i) {
		if (*limit == 0)
			break;
		struct day_item *day = VECTOR_NTH(&day_items, i);

		switch (day->type) {
		case APPT:
			print_apoint(fmt_apt, date, day->item.apt);
			break;
		case EVNT:
			print_event(fmt_ev, date, day->item.ev);
			break;
		case RECUR_APPT:
			print_recur_apoint(fmt_rapt, date, day->start,
					   day->item.rapt);
			break;
		case RECUR_EVNT:
			print_recur_event(fmt_rev, date, day->item.rev);
			break;
		default:
			EXIT(_("unknown item type"));
			/* NOTREACHED */
		}
		(*limit)--;
	}
}

/*
 * Store events and appointments for a range of days in the day vector -
 * beginning with the selected day - and load them into the APP listbox. If no
 * day-change occurs, reset the selected APP item and with it the selected day,
 * thereby storing and loading the same range of days.
 */
void day_do_storage(int day_changed)
{
	int pre_sel;
	/*
	 * Save the selected item before rebuilding the day vector -
	 * unless a preselection is already set.
	 */
	if(!(pre_sel = day_check_sel_data()))
		day_set_sel_data(ui_day_get_sel());

	if (!day_changed)
		ui_day_sel_reset();

	/* The day_items vector. */
	day_store_items(get_slctd_day(), 1, day_get_days());
	/* The APP listbox. */
	ui_day_load_items();

	if (day_changed && !pre_sel)
		ui_day_sel_reset();
	else
		ui_day_find_sel();

	day_set_sel_data(&empty_day);
}

/* Display an item inside a popup window. */
void day_popup_item(struct day_item *day)
{
	const char *note_heading = _("Note:");
	size_t note_size = 3500;

	if (day->type == EVNT || day->type == RECUR_EVNT) {
		if (day_item_get_note(day)) {
			char note[note_size];
			char *notepath, *msg;
			FILE *fp;

			asprintf(&notepath, "%s%s", path_notes, day_item_get_note(day));
			fp = fopen(notepath, "r");
			if (fp == NULL) {
				item_in_popup(NULL, NULL, day_item_get_mesg(day), _("Event:"));
				return;
			}
			note_read_contents(note, note_size, fp);
			fclose(fp);
			mem_free(notepath);

			asprintf(&msg, "%s\n\n%s\n%s", day_item_get_display_mesg(day), note_heading, note);
			item_in_popup(NULL, NULL, msg, _("Event:"));
			mem_free(msg);
		} else {
			item_in_popup(NULL, NULL, day_item_get_display_mesg(day), _("Event:"));
		}
	} else if (day->type == APPT || day->type == RECUR_APPT) {
		char a_st[100], a_end[100];

		/* FIXME: Redesign apoint_sec2str() and remove the need for a temporary
		 * appointment item here. */
		struct apoint apt_tmp;
		apt_tmp.start = day->start;
		apt_tmp.dur = day_item_get_duration(day);
		apoint_sec2str(&apt_tmp, ui_day_sel_date(), a_st, a_end);

		if (day_item_get_note(day)) {
			char note[note_size];
			char *notepath, *msg;
			FILE *fp;

			asprintf(&notepath, "%s%s", path_notes, day_item_get_note(day));
			fp = fopen(notepath, "r");
			if (fp == NULL) {
				item_in_popup(a_st, a_end, day_item_get_mesg(day), _("Appointment:"));
				return;
			}
			note_read_contents(note, note_size, fp);
			fclose(fp);
			mem_free(notepath);

			asprintf(&msg, "%s\n\n%s\n%s", day_item_get_display_mesg(day), note_heading, note);
			item_in_popup(a_st, a_end, msg, _("Appointment:"));
			mem_free(msg);
		} else {
			item_in_popup(a_st, a_end, day_item_get_display_mesg(day), _("Appointment:"));
		}
	} else {
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}
}

/*
 * Check whether there is an item on a given day and return the colour
 * attribute for the item:
 * ATTR_TRUE if the selected day contains a regular event or appointment,
 * ATTR_LOW if the selected day does not contain a regular event or
 * appointment but an occurrence of a recurrent item. Returns 0 otherwise.
 */
int day_check_if_item(struct date day)
{
	const time_t t = date2sec(day, 0, 0);

	if (LLIST_FIND_FIRST(&eventlist, (time_t *)&t, event_inday))
		return ATTR_TRUE;

	LLIST_TS_LOCK(&alist_p);
	if (LLIST_TS_FIND_FIRST(&alist_p, (time_t *)&t, apoint_inday)) {
		LLIST_TS_UNLOCK(&alist_p);
		return ATTR_TRUE;
	}
	LLIST_TS_UNLOCK(&alist_p);

	if (LLIST_FIND_FIRST(&recur_elist, (time_t *)&t, recur_event_inday))
		return ATTR_LOW;

	LLIST_TS_LOCK(&recur_alist_p);
	if (LLIST_TS_FIND_FIRST(&recur_alist_p, (time_t *)&t,
				recur_apoint_inday)) {
		LLIST_TS_UNLOCK(&recur_alist_p);
		return ATTR_LOW;
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	return 0;
}

static unsigned fill_slices(int *slices, int slicesno, int first, int last)
{
	int i;

	if (first < 0 || last < first)
		return 0;

	if (last >= slicesno)
		last = slicesno - 1;	/* Appointment spanning more than one day. */

	for (i = first; i <= last; i++)
		slices[i] = 1;

	return 1;
}

/*
 * Fill in the 'slices' vector given as an argument with 1 if there is an
 * appointment in the corresponding time slice, 0 otherwise.
 * A 24 hours day is divided into 'slicesno' number of time slices.
 */
unsigned day_chk_busy_slices(struct date day, int slicesno, int *slices)
{
	const time_t t = date2sec(day, 0, 0);
	llist_item_t *i;
	int slicelen;

	slicelen = DAYINSEC / slicesno;

#define  SLICENUM(tsec)  ((tsec) / slicelen % slicesno)

	LLIST_TS_LOCK(&recur_alist_p);
	LLIST_TS_FIND_FOREACH(&recur_alist_p, (time_t *)&t,
			      recur_apoint_inday, i) {
		struct recur_apoint *rapt = LLIST_TS_GET_DATA(i);
		time_t occurrence;
		time_t start, end;

		if (!recur_apoint_find_occurrence(rapt, t, &occurrence))
			continue;

		if (occurrence >= t)
			start = get_item_time(occurrence);
		else
			start = 0;

		if (occurrence + rapt->dur < t + DAYINSEC)
			end = get_item_time(occurrence + rapt->dur);
		else
			end = DAYINSEC - 1;

		/*
		 * If an item ends on 12:00, we do not want the 12:00 slot to
		 * be marked busy.
		 */
		if (end > start)
			end--;

		if (!fill_slices(slices, slicesno, SLICENUM(start),
					SLICENUM(end))) {
			LLIST_TS_UNLOCK(&recur_alist_p);
			return 0;
		}
	}
	LLIST_TS_UNLOCK(&recur_alist_p);

	LLIST_TS_LOCK(&alist_p);
	LLIST_TS_FIND_FOREACH(&alist_p, (time_t *)&t, apoint_inday, i) {
		struct apoint *apt = LLIST_TS_GET_DATA(i);
		time_t start = get_item_time(apt->start);
		time_t end = get_item_time(apt->start + apt->dur);

		if (apt->start >= t + DAYINSEC)
			break;
		if (apt->start < t)
			start = 0;
		if (apt->start + apt->dur >= t + DAYINSEC)
			end = DAYINSEC - 1;

		/*
		 * If an item ends on 12:00, we do not want the 12:00 slot to
		 * be marked busy.
		 */
		if (end > start)
			end--;

		if (!fill_slices(slices, slicesno, SLICENUM(start),
					SLICENUM(end))) {
			LLIST_TS_UNLOCK(&alist_p);
			return 0;
		}
	}
	LLIST_TS_UNLOCK(&alist_p);

#undef SLICENUM
	return 1;
}

/* Cut an item so it can be pasted somewhere else later. */
struct day_item *day_cut_item(int item_number)
{
	struct day_item *p = day_get_item(item_number);

	switch (p->type) {
	case EVNT:
		event_delete(p->item.ev);
		break;
	case RECUR_EVNT:
		recur_event_erase(p->item.rev);
		break;
	case APPT:
		apoint_delete(p->item.apt);
		break;
	case RECUR_APPT:
		recur_apoint_erase(p->item.rapt);
		break;
	default:
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}

	return p;
}

/* Paste a previously cut item. */
int day_paste_item(struct day_item *p, time_t date)
{
	if (!p->type) {
		/* No previously cut item. */
		return 0;
	}
	/*
	 * Valid until date of recurrent items?
	 * Careful: p->start is not yet set.
	 */
	time_t until;

	switch (p->type) {
	case EVNT:
		event_paste_item(p->item.ev, date);
		break;
	case RECUR_EVNT:
		/* want: until = shift + old_until */
		if (p->item.rev->rpt->until &&
		    overflow_add(
			date - p->item.rev->day,
			p->item.rev->rpt->until,
			&until)
		)
			return 0;
		if (check_sec(&until))
			recur_event_paste_item(p->item.rev, date);
		else
			return 0;
		break;
	case APPT:
		apoint_paste_item(p->item.apt, date);
		break;
	case RECUR_APPT:
		/* wanted: until = shift + old_until */
		if (p->item.rapt->rpt->until &&
		    overflow_add(
			date - DAY(p->item.rapt->start),
			p->item.rapt->rpt->until,
			&until)
		)
			return 0;
		if (check_sec(&until))
			recur_apoint_paste_item(p->item.rapt, date);
		else
			return 0;
		break;
	default:
		EXIT(_("unknown item type"));
		/* NOTREACHED */
	}

	return p->type;
}

/* Returns a structure containing the selected item. */
struct day_item *day_get_item(int item_number)
{
	return VECTOR_NTH(&day_items, item_number);
}

unsigned day_item_count(int include_captions)
{
	return (include_captions ? VECTOR_COUNT(&day_items) : day_items_nb);
}

/* Attach a note to an appointment or event. */
void day_edit_note(struct day_item *p, const char *editor)
{
	char *note;

	note = day_item_get_note(p);
	edit_note(&note, editor);

	switch (p->type) {
	case RECUR_EVNT:
		p->item.rev->note = note;
		break;
	case EVNT:
		p->item.ev->note = note;
		break;
	case RECUR_APPT:
		p->item.rapt->note = note;
		break;
	case APPT:
		p->item.apt->note = note;
		break;
	default:
		break;
	}
}

/* View a note previously attached to an appointment or event */
void day_view_note(struct day_item *p, const char *pager)
{
	view_note(day_item_get_note(p), pager);
}

/* Switch notification state for an item. */
void day_item_switch_notify(struct day_item *p)
{
	switch (p->type) {
	case RECUR_APPT:
		recur_apoint_switch_notify(p->item.rapt);
		break;
	case APPT:
		apoint_switch_notify(p->item.apt);
		break;
	default:
		break;
	}
}
