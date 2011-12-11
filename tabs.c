#include "tabbar.h"
#include "appres.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void set_mainw_name(char *);

/* this wid is not the tab's window id (tabs are not widgets)
   it is instead id of the window the tab represents
 */
int find_tab_by_wid(Window w)
{
	pw_tab_group * tg = &the_bar.tab_group; 
	int i;
	for (i = 0; i < tg->tab_count; i++) {
		if (tg->tabs[i].w == w)
			return i;
	}
	return -1;
}

int calc_tab_size()
{
	pw_tab_group * tg = &the_bar.tab_group;
	int font_width = tab_options.font->max_bounds.width;
	tg->label_x_offset = font_width * 2;
	tg->height = tab_options.font->ascent + tab_options.font->descent
		+ button_params.margin_top + button_params.padding_top
		+ button_params.margin_bottom + button_params.padding_bottom -1;

	tg->label_y_offset = tab_options.font->ascent + button_params.margin_top
		+ button_params.padding_top;

	/*
	  important: determine label_text_len
	  tabs are between 6 and 32 chars long; fit as many as we can, if
	  all can fit, make as long as we can
	 */
	int min_len = 8+4;
	int max_len = TAB_LABEL_LEN + 3;

	int total_char_len = tg->width / font_width;
	int label_char_len;
	if (total_char_len > tg->tab_count * min_len) {
		if (tg->tab_count)
			label_char_len = total_char_len / tg->tab_count;
		else
			label_char_len = total_char_len;

		if (label_char_len > max_len)
			label_char_len = max_len;
	}
	else {
		label_char_len = min_len;
	}
	int wedge_width = tg->height/3;

	tg->label_text_len = label_char_len - 4;
	tg->tab_width = (label_char_len) * font_width;

	if (tab_options.tabs_on_top) {
		tg->node[0].y = tg->height-1;
		tg->node[1].x = tg->tab_width + wedge_width;
		tg->node[1].y = 0;
		tg->node[2].x = -wedge_width;
		tg->node[2].y = 1-tg->height;
		tg->node[3].x = wedge_width - tg->tab_width;
		tg->node[3].y = 0;
		tg->node[4].x = -wedge_width;
		tg->node[4].y = tg->height - 1;
	}
	else{
		tg->node[0].y = 0;
		tg->node[1].x = tg->tab_width + wedge_width;
		tg->node[1].y = 0;
		tg->node[2].x = -wedge_width;
		tg->node[2].y = tg->height-1;
		tg->node[3].x = wedge_width - tg->tab_width;
		tg->node[3].y = 0;
		tg->node[4].x = -wedge_width;
		tg->node[4].y = - tg->height+1;
	}
	return 1;
}

int
init_tab_group(int x, int y, int width, int height)
{
	pw_tab_group * tg = &the_bar.tab_group;
	tg->active = 0;
	tg->x = x;
	tg->y = y;
	tg->width = width;
	tg->scroll_offset = 0;
	tg->tab_count = 0;
	calc_tab_size();
	return 1;
}

int realize_tab_group(Window parent) {
	XGCValues values;
	pw_tab_group * tg = &the_bar.tab_group;
	tg->win = XCreateSimpleWindow(gdi.display, parent,
				      tg->x, tg->y,
				      tg->width, tg->height,
				      0, 0, tab_options.bg);
	tg->gc = XCreateGC(gdi.display, tg->win, 0, &values);
	XSelectInput(gdi.display, tg->win, ButtonPressMask | ExposureMask);
	XMapWindow(gdi.display, tg->win);
	return 1;
}

int add_tab(Window w)
{
	char * name;
	pw_tab_group * tg = &the_bar.tab_group;
	static int tab_hist = 0;

	if (tg->tab_count >= MAX_TABS)
		return 0;
	tab_t * nt = tg->tabs + tg->tab_count;
	sprintf(nt->label, "tab %d", ++tab_hist);
	if (XFetchName(gdi.display, w, &name)
	    && name) {
		tab_set_name(tg->tab_count, name);
		XFree(name);
	}
	nt->w = w;
	XSelectInput(gdi.display, w, PropertyChangeMask);
	tg->tab_count ++;
	calc_tab_size();
	return tg->tab_count;
}

int remove_tab(Window w)
{
	pw_tab_group * tg = &the_bar.tab_group; 
	int i;
	for (i = 0; i < tg->tab_count; i++) {
		if (tg->tabs[i].w == w)
			break;
	}
	if (i >= tg->tab_count) /* not found */
		return 0;
	memmove(tg->tabs + i, tg->tabs + i + 1,
		(tg->tab_count - i - 1) * sizeof(tab_t));
	tg->tab_count --;
	if (tg->active >= tg->tab_count)
		tg->active = tg->tab_count-1;
	set_active_tab(tg->active);
	calc_tab_size();
	/* need to repaint window here */
	return 1;
}

int resize_tab_group(int w, int h)
{
	pw_tab_group * tg = &the_bar.tab_group;
	XResizeWindow(gdi.display, tg->win, w, h);
	tg->width = w;
	tg->height = h;
	/* set tab width */
	calc_tab_size();
	/* repaint window? */
	return 1;
}

int move_tab_group(int x, int y)
{
	pw_tab_group * tg = &the_bar.tab_group; 
	XMoveWindow(gdi.display, tg->win, x, y);
	tg->x = x;
	tg->y = y;
	/* repaint? */
	return 1;
}

int draw_tabs(int force)
{
	int i;
	unsigned long fg, bg, bd;
	pw_tab_group * tg = &the_bar.tab_group; 

	if (force)
		XClearWindow(gdi.display, tg->win);
	int j;
	for (j = 0; j <= tg->tab_count; j++) {
		/* this ugly chunk is to make sure active tab get drawn
		   last, thus on top of other tabs */
		if (j == tg->active)
			continue;
		if (j == tg->tab_count) {
			if (tg->active < 0)
				continue;
			i = tg->active;
			fg = tab_options.active_fg;
			bg = tab_options.active_bg;
			bd = tab_options.active_border;
		}
		else {
			i = j;
			fg = tab_options.tab_fg;
			bg = tab_options.tab_bg;
			bd = tab_options.tab_border;
		}
		tg->node[0].x = i * tg->tab_width - tg->scroll_offset;
		if (tg->node[0].x >= tg->width || tg->node[0].x + tg->tab_width < 0)
			continue;

		XSetForeground(gdi.display, tg->gc, bg);
		XFillPolygon(gdi.display, tg->win, tg->gc, tg->node, 4, Convex, CoordModePrevious);

		XSetForeground(gdi.display, tg->gc, bd);
		XDrawLines(gdi.display, tg->win, tg->gc, tg->node,
			   5, CoordModePrevious);

		XSetForeground(gdi.display, tg->gc, fg);
		XSetFont(gdi.display, tg->gc, tab_options.font->fid);
		int len = strlen(tg->tabs[i].label);
		if (len > tg->label_text_len)
			len = tg->label_text_len;
		XDrawString(gdi.display, tg->win, tg->gc,
			    tg->node[0].x + tg->label_x_offset,
			    tg->label_y_offset,
			    tg->tabs[i].label, len);
	}
	return 1;
}

int get_tab_offset(int idx) {
	pw_tab_group * tg = &the_bar.tab_group;
	return tg->tab_width * idx;
}

int set_active_tab(int idx)
{
	pw_tab_group * tg = &the_bar.tab_group; 
	XMapWindow(gdi.display, tg->tabs[idx].w);
	if (tg->active < tg->tab_count
		&& tg->active != idx)
		XUnmapWindow(gdi.display, tg->tabs[tg->active].w);
	tg->active = idx;

	int active_offset = get_tab_offset(tg->active);

	if (active_offset > tg->scroll_offset + tg->width - tg->tab_width) {
		tg->scroll_offset = active_offset - tg->width + tg->tab_width;
	}
	else if (active_offset < tg->scroll_offset) {
		tg->scroll_offset = active_offset;
	}
	draw_tabs(0);
	XSetInputFocus(gdi.display, tg->tabs[idx].w, RevertToPointerRoot, CurrentTime);
	set_mainw_name(tg->tabs[tg->active].label);
	return idx;
}

int tab_click(int x, int y)
{
	pw_tab_group * tg = &the_bar.tab_group; 
	int clicked = (x + tg->scroll_offset) / tg->tab_width;
	if (clicked < 0
	    || clicked >= tg->tab_count
	    || clicked == tg->active)
		return 0;
	return set_active_tab(clicked);
}

int tab_set_name(int idx, char * name)
{
	pw_tab_group * tg = &the_bar.tab_group; 
	strncpy(tg->tabs[idx].label, name, TAB_LABEL_LEN);
	draw_tabs(0);
	return 1;
}

