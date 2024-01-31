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

#include "calcurse.h"
#include <ctype.h>

static unsigned ui_todo_view;

static struct todo *ui_todo_selitem(void)
{
	return todo_get_item(listbox_get_sel(&lb_todo),
			     ui_todo_view == TODO_HIDE_COMPLETED_VIEW);
}

static void ui_todo_set_selitem(struct todo *todo)
{
	int n = todo_get_position(todo,
				  ui_todo_view == TODO_HIDE_COMPLETED_VIEW);
	if (n >= 0)
		listbox_set_sel(&lb_todo, n);
}

/* Request user to enter a new todo item. */
void ui_todo_add(void)
{
	int ch;
	const char *mesg = _("Enter the new TODO item:");
	const char *mesg_id =
	    _("Enter the TODO priority [0 (none), 1 (highest) - 9 (lowest)]:");
	char todo_input[BUFSIZ] = "";

	status_mesg(mesg, "");
	if (getstring(win[STA].p, todo_input, BUFSIZ, 0, 1) ==
	    GETSTRING_VALID) {
		do {
			status_mesg(mesg_id, "");
			ch = keys_wgetch(win[KEY].p);
			if (ch == RETURN)
				ch = '0';
			else if (ch == ESCAPE)
				return;
		} while (!isdigit(ch));
		struct todo *todo = todo_add(todo_input, ch - '0', 0, NULL);
		ui_todo_load_items();
		io_set_modified();
		ui_todo_set_selitem(todo);
	}
}

/* Delete an item from the TODO list. */
void ui_todo_delete(void)
{
	const char *del_todo_str =
	    _("Do you really want to delete this task?");
	const char *erase_warning =
	    _("This item has a note attached to it. "
	      "Delete (t)odo or just its (n)ote?");
	const char *erase_choice = _("[tn]");
	const int nb_erase_choice = 2;
	int answer;

	struct todo *item = ui_todo_selitem();

	if (!item || (conf.confirm_delete &&
		      (status_ask_bool(del_todo_str) != 1))) {
		wins_erase_status_bar();
		return;
	}

	if (item->note)
		answer = status_ask_choice(erase_warning, erase_choice,
					   nb_erase_choice);
	else
		answer = 1;

	switch (answer) {
	case 1:
		todo_delete(item);
		ui_todo_load_items();
		io_set_modified();
		break;
	case 2:
		todo_delete_note(item);
		io_set_modified();
		break;
	default:
		wins_erase_status_bar();
		return;
	}
}

/* Edit the description of an already existing todo item. */
void ui_todo_edit(void)
{
	struct todo *item = ui_todo_selitem();
	const char *mesg = _("Enter the new TODO description:");

	if (!item)
		return;

	status_mesg(mesg, "");
	updatestring(win[STA].p, &item->mesg, 0, 1);
	todo_resort(item);
	ui_todo_load_items();
	io_set_modified();
	ui_todo_set_selitem(item);
}

/* Pipe a todo item to an external program. */
void ui_todo_pipe(void)
{
	char cmd[BUFSIZ] = "";
	char const *arg[] = { cmd, NULL };
	int pout;
	int pid;
	FILE *fpout;

	struct todo *item = ui_todo_selitem();

	if (!item)
		return;

	status_mesg(_("Pipe item to external command:"), "");
	if (getstring(win[STA].p, cmd, BUFSIZ, 0, 1) != GETSTRING_VALID)
		return;

	wins_prepare_external();
	if ((pid = shell_exec(NULL, &pout, NULL, 0, *arg, arg))) {
		fpout = fdopen(pout, "w");
		todo_write(item, fpout);
		fclose(fpout);
		child_wait(NULL, &pout, NULL, pid);
		press_any_key();
	}
	wins_unprepare_external();
}

/* Display todo items in the corresponding panel. */
void ui_todo_draw(int n, WINDOW *win, int y, int hilt, void *cb_data)
{
	llist_item_t *i = *((llist_item_t **)cb_data);
	struct todo *todo = LLIST_TS_GET_DATA(i);
	char mark[] = { 0, 0, 0, 0 };
	int width = lb_todo.sw.w - 2;
	char buf[width * UTF8_MAXLEN];
	char *mesg;
	int j;

	if (ui_todo_view == TODO_HIDE_COMPLETED_VIEW) {
		while (i && todo->completed) {
			i = i->next;
			if (i)
				todo = LLIST_TS_GET_DATA(i);
		}
	}

	mark[0] = todo->completed ? 'X' : (todo->id > 0 ? '0' + todo->id : 0);
	if (todo->note) {
		if (mark[0] == '\0') {
			mark[0] = '>';
			mark[1] = ' ';
		} else {
			mark[1] = '>';
			mark[2] = ' ';
		}
	} else if (mark[0] != '\0') {
		mark[1] = '.';
		mark[2] = ' ';
	}
	width -= strlen(mark);

	hilt = hilt && (wins_slctd() == TOD);

	if (hilt)
		custom_apply_attr(win, ATTR_HIGHEST);

	mesg = todo->mesg;
	if (mesg[0] == '\0')
		mesg = EMPTY_EVENT_DESC_DEFAULT;

	if (utf8_strwidth(mesg) >= width) {
		width -= 3;
		for (j = 0; mesg[j] && width > 0; j++) {
			if (!UTF8_ISCONT(mesg[j]))
				width -= utf8_width(&mesg[j]);
			buf[j] = mesg[j];
		}
		if (j) {
			buf[j - 1] = '.';
			buf[j] = '.';
			buf[j + 1] = '.';
			buf[j + 2] = '\0';
		} else {
			buf[0] = 0;
		}
		mesg = buf;
	}

	mvwprintw(win, y, 0, "%s%s", mark, mesg);

	if (hilt)
		custom_remove_attr(win, ATTR_HIGHEST);

	*((llist_item_t **)cb_data) = i->next;
}

enum listbox_row_type ui_todo_row_type(int i, void *cb_data)
{
	return LISTBOX_ROW_TEXT;
}

int ui_todo_height(int n, void *cb_data)
{
	return 1;
}

void ui_todo_load_items(void)
{
	int n = 0;
	llist_item_t *i;

	/* TODO: Optimize this by keeping the list size in a variable. */
	LLIST_FOREACH(&todolist, i) {
		struct todo *todo = LLIST_TS_GET_DATA(i);
		if (ui_todo_view == TODO_HIDE_COMPLETED_VIEW &&
		    todo->completed)
			continue;
		n++;
	}

	listbox_load_items(&lb_todo, n);
}

void ui_todo_sel_reset(void)
{
	listbox_sel_move(&lb_todo, 0);
}

void ui_todo_sel_move(int delta)
{
	listbox_sel_move(&lb_todo, delta);
}

/* Updates the TODO panel. */
void ui_todo_update_panel(int hilt)
{
	/*
	 * This is used and modified by ui_todo_draw() to avoid quadratic
	 * running time.
	 */
	llist_item_t *p = LLIST_FIRST(&todolist);

	listbox_set_cb_data(&lb_todo, &p);
	listbox_display(&lb_todo, hilt);
}

/* Change an item priority by pressing '+' or '-' inside TODO panel. */
void ui_todo_chg_priority(int diff)
{
	struct todo *item = ui_todo_selitem();

	if (!item)
		return;

	int id = item->id + diff;
	struct todo *item_new;

	if (id < 0)
		id = 0;
	else if (id > 9)
		id = 9;

	item_new = todo_add(item->mesg, id, item->completed, item->note);
	todo_delete(item);
	io_set_modified();
	ui_todo_set_selitem(item_new);
}

void ui_todo_popup_item(void)
{
	struct todo *item = ui_todo_selitem();

	if (!item)
		return;

	if (item->note) {
		/* Assign a sane default note size that will cleanly
		 * truncate long notes */
		const char *note_heading = _("Note:");
		size_t note_size = 3500;
		char note[note_size];
		char *notepath, *msg;
		FILE *fp;

		asprintf(&notepath, "%s%s", path_notes, item->note);
		fp = fopen(notepath, "r");
		if (fp == NULL) {
			item_in_popup(NULL, NULL, item->mesg, _("TODO:"));
			return;
		}

		note_read_contents(note, note_size, fp);
		fclose(fp);
		mem_free(notepath);

		asprintf(&msg, "%s\n\n%s\n%s", item->mesg, note_heading, note);
		item_in_popup(NULL, NULL, msg, _("TODO:"));
		mem_free(msg);
	} else {
		item_in_popup(NULL, NULL, item->mesg, _("TODO:"));
	}
}

void ui_todo_flag(void)
{
	struct todo *item = ui_todo_selitem();

	if (!item)
		return;

	todo_flag(item);
	ui_todo_load_items();
	io_set_modified();
	ui_todo_set_selitem(item);
}

void ui_todo_view_note(void)
{
	struct todo *item = ui_todo_selitem();

	if (!item)
		return;

	todo_view_note(item, conf.pager);
}

void ui_todo_edit_note(void)
{
	struct todo *item = ui_todo_selitem();

	if (!item)
		return;

	todo_edit_note(item, conf.editor);
	io_set_modified();
}

/* Switch to next todo view. */
void ui_todo_view_next(void)
{
	ui_todo_view++;
	if (ui_todo_view == TODO_VIEWS)
		ui_todo_view = 0;
	ui_todo_load_items();
}

/* Switch to previous todo view. */
void ui_todo_view_prev(void)
{
	if (ui_todo_view == 0)
		ui_todo_view = TODO_VIEWS;
	ui_todo_view--;
	ui_todo_load_items();
}

void ui_todo_set_view(int view)
{
	ui_todo_view = (view < 0 || view >= TODO_VIEWS) ?
		       TODO_SHOW_COMPLETED_VIEW : view;
}

int ui_todo_get_view(void)
{
	return (int)ui_todo_view;
}
