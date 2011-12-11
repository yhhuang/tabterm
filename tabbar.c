#include "tabbar.h"
#include "appres.h"

#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>

#include <stdlib.h>
#include <string.h>

tabbar_t the_bar;

extern void spawn_xterm();
extern void set_mainw_name(char * buf);

key_binding_t key_bindings[MAX_KEYBINDS] = {
	{"New",		0, 0, spawn_xterm},
	{"Left",	0, 0, shift_bar_left},
	{"Right",	0, 0, shift_bar_right},
	{"Attach",	0, 0, swallow_window},
	{"Detach",	0, 0, release_window},
	{"Kill",	0, 0, kill_active_tab},
	{"Next",	0, 0, next_tab},
	{"Prev",	0, 0, prev_tab}
};
/*  Oooh, weird.
    XCharStruct has members width, lbearing and rbearing, but sounds like
    it should be width = lbearing + rbearing. Where the hell is the bounding
    box for the glyphs without white space?
 */

void kill_active_tab()
{
	if (!the_bar.tab_group.tab_count)
		return;
	XDestroyWindow(gdi.display, the_bar.tab_group.tabs[the_bar.tab_group.active].w);
}

void clamp_tabs()
{
	if (the_bar.tab_group.scroll_offset + the_bar.tab_group.width > 
	    the_bar.tab_group.tab_width * the_bar.tab_group.tab_count)
		the_bar.tab_group.scroll_offset =
			the_bar.tab_group.tab_width * the_bar.tab_group.tab_count
			- the_bar.tab_group.width;
	if (the_bar.tab_group.scroll_offset < 0)
		the_bar.tab_group.scroll_offset = 0;
}

void shift_bar_left()
{
	if (!the_bar.tab_group.tab_count)
		return;
	the_bar.tab_group.scroll_offset += the_bar.tab_group.tab_width*2/3;
	clamp_tabs();
	draw_tabs(1);
}

void shift_bar_right()
{
	if (!the_bar.tab_group.tab_count)
		return;
	the_bar.tab_group.scroll_offset -= the_bar.tab_group.tab_width*2/3;
	clamp_tabs();
	draw_tabs(1);
}

int
init_tabbar(int width) /* height is auto */
{
	the_bar.width = width;

	set_button_params();
	init_button_group(&the_bar.left_btns);
	init_button_group(&the_bar.right_btns);

	add_button_to_group(&the_bar.left_btns, create_button('+', spawn_xterm));
	add_button_to_group(&the_bar.left_btns, create_button('<', shift_bar_left));
	add_button_to_group(&the_bar.left_btns, create_button('>', shift_bar_right));

	add_button_to_group(&the_bar.right_btns, create_button('A', swallow_window));
	add_button_to_group(&the_bar.right_btns, create_button('D', release_window));
	add_button_to_group(&the_bar.right_btns, create_button('X', kill_active_tab));

	the_bar.height = the_bar.left_btns.height;

	init_tab_group(0, the_bar.left_btns.width,
		       the_bar.width - the_bar.left_btns.width - the_bar.right_btns.width,
		       the_bar.height);
	if (the_bar.tab_group.height > the_bar.height) {
		the_bar.height = the_bar.tab_group.height;
	}
	the_bar.left_btns.height = the_bar.height;
	the_bar.right_btns.height = the_bar.height;
	the_bar.right_btns.y = the_bar.left_btns.y;
	the_bar.tab_group.height = the_bar.height;
	return 1;
}

int
realize_tab_bar(Window parent)
{
	the_bar.win = XCreateSimpleWindow(gdi.display, parent,
					  the_bar.x, the_bar.y,
					  the_bar.width, the_bar.height,
					  0, 0, tab_options.bg);

	the_bar.left_btns.x = 0;
	the_bar.left_btns.y = 0;
	realize_button_group(&the_bar.left_btns, the_bar.win);

	the_bar.right_btns.x = the_bar.width - the_bar.right_btns.width;
	the_bar.right_btns.y = 0;
	realize_button_group(&the_bar.right_btns, the_bar.win);

	the_bar.tab_group.x = the_bar.left_btns.width;
	the_bar.tab_group.y = 0;
	realize_tab_group(the_bar.win);

	XSelectInput(gdi.display, the_bar.win, EnterWindowMask);
	XMapWindow(gdi.display, the_bar.win);
	the_bar.realized = 1;
	return 1;
}

int set_bar_geometry(tabbar_t * tb, int x, int y, int width)
{
	XMoveResizeWindow(gdi.display, the_bar.win, x, y, width, the_bar.height);
	the_bar.x = x; the_bar.y = y; the_bar.width = width;
	move_button_group(&the_bar.right_btns, width - the_bar.right_btns.width);
	resize_tab_group(width - the_bar.right_btns.width - the_bar.left_btns.width,
			 the_bar.height);
	return 1;
}

int new_tab(Window w)
{
	add_tab(w);
	set_active_tab(the_bar.tab_group.tab_count -1);
	draw_tabs(1);
	return 1;
}

void next_tab()
{
	if (the_bar.tab_group.active +1 < the_bar.tab_group.tab_count)
		set_active_tab(the_bar.tab_group.active + 1);
	else
		set_active_tab(0);
	draw_tabs(1);
}

void prev_tab()
{
	if (the_bar.tab_group.active > 0)
		set_active_tab(the_bar.tab_group.active - 1);
	else
		set_active_tab(the_bar.tab_group.tab_count - 1);
	draw_tabs(1);
}

int delete_tab(Window w)
{
	remove_tab(w);
//	if (the_bar.tab_group.active >= the_bar.tab_group.tab_count)
//		set_active_tab(the_bar.tab_group, the_bar.tab_group -> tab_count -1);
	clamp_tabs();
	draw_tabs(1);
	return the_bar.tab_group.tab_count;
}

Window get_active_term(tabbar_t * tb)
{
	return the_bar.tab_group.tabs[the_bar.tab_group.active].w;
}

int bar_handle_expose(XExposeEvent * ee)
{
	pw_button * btn = 0;
	if (ee->window == the_bar.tab_group.win){
		draw_tabs(0);
		return 1;
	}
	else if ((btn = find_button_by_wid(ee->window, &the_bar.left_btns))
		 ||(btn = find_button_by_wid(ee->window, &the_bar.right_btns)) )
	{
		paint_button(btn);
		return 1;
	}
	else 
		return 0;
}

int bar_handle_button(XButtonEvent * xbtn)
{
	pw_button * btn = 0;
/*	printf("button %d\n", xbtn->button); */
	switch (xbtn->button) {
	case 1:
		if (xbtn->window == the_bar.tab_group.win){
			if (tab_click(xbtn->x, xbtn->y))
				draw_tabs(0);
		}
		else if ((btn = find_button_by_wid(xbtn->window, &the_bar.right_btns))
			||(btn = find_button_by_wid(xbtn->window, &the_bar.left_btns)) )
		{
			process_button_event(btn, xbtn);
		}
		break;
	case 2:
	case 3:
		break;
	case 4:
		shift_bar_right();
		break;
	case 5:
		shift_bar_left();
		break;
	default:
		break;
	}
	return 1;
}

int resize_term_windows(int width, int height)
{
	int i;
	for (i = 0; i < the_bar.tab_group.tab_count; i++){
		XMoveResizeWindow(gdi.display, the_bar.tab_group.tabs[i].w,
			      0, tab_options.tabs_on_top?the_bar.height:0,
			      width, height);
	}
	set_bar_geometry(&the_bar, 0, 
			 tab_options.tabs_on_top?0:(gdi.height - the_bar.height),
			 gdi.width);
	return 1;
}

int bar_handle_prop(XEvent * evt)
{
	XPropertyEvent * xpe = &(evt->xproperty);
	int tabid = find_tab_by_wid(xpe->window);
	if (tabid < 0)
		return 0;
	char * aname = XGetAtomName(gdi.display, xpe->atom);
	char * name = 0;
#ifdef DEBUG
	fprintf(stderr, "prop: %s\n", aname);
#endif
	if (aname) {
		if (!strncmp(aname, "WM_NAME", 7)
		    && XFetchName(gdi.display, xpe->window, &name)
		    && name)
		{
			tab_set_name(tabid, name);
			if (tabid == the_bar.tab_group.active) {
				set_mainw_name(name);
			}
		}
		XFree(aname);
	}

	return 1;
}

void set_focus()
{
	return;
	if (the_bar.tab_group.tab_count) {
		Window w = the_bar.tab_group.tabs[the_bar.tab_group.active].w;
		XWindowAttributes attr;
		XGetWindowAttributes(gdi.display, w, &attr);
		if (attr.map_state == IsViewable) {
			XSetInputFocus(gdi.display, w, RevertToPointerRoot,
				       CurrentTime);
		}
	}
}

#define MAX_PROSPECT_CHILDREN 16
Window new_children[MAX_PROSPECT_CHILDREN] = {0L};

int
expect_child(Window w)
{
	int i;
	for (i = 0; i < MAX_PROSPECT_CHILDREN; i++) {
		if (new_children[i] == 0) {
			new_children[i] = w;
			return 1;
		}
	}
	return 0;
}

int
received_child(Window w)
{
	int i;
	for (i = 0; i < MAX_PROSPECT_CHILDREN; i++) {
		if (new_children[i] == w) {
			new_children[i] = 0;
			return 1;
		}
	}
	return 0;
}

int is_tabbed_window(Window w)
{
	int i;
	for (i = 0; i< the_bar.tab_group.tab_count; i++)
		if (w == the_bar.tab_group.tabs[i].w)
			return 1;
	return 0;
}

int is_expected_child(Window w)
{
	int i;
	for (i = 0; i < MAX_PROSPECT_CHILDREN; i++) {
		if (new_children[i] == w) {
			new_children[i] = w;
			return 1;
		}
	}
	return 0;
}

Window get_parent(Window w)
{
	Window parent, root, *children;
	unsigned int nchildren;
	if (XQueryTree(gdi.display, w, &root, &parent,
		       &children, &nchildren)) {
		XFree(children);
		return parent;
	}
	return None;
}

void swallow_window()
{
	Window w = Select_Window();
	if (w == gdi.root) {
		fprintf(stderr, "refuse to put root as a child window\n");
		return;
	}
	w = XmuClientWindow (gdi.display, w);
	if (w == None || w == gdi.root || w == gdi.mainw)
		return;
	if (expect_child(w))
		XReparentWindow(gdi.display, w, gdi.mainw, 0, 0);
}

void release_window()
{
	tab_t * t = the_bar.tab_group.tabs + the_bar.tab_group.active;
	XReparentWindow(gdi.display, t->w, gdi.root, 0, 0);
	delete_tab(t->w);
}

Window Select_Window()
{
	int status;
	Cursor cursor;
	XEvent event;
	Display * dpy = gdi.display;
	Window target_win = None, root=gdi.root;
	int buttons = 0;

	/* Make the target cursor */
	cursor = XCreateFontCursor(dpy, XC_crosshair);

	/* Grab the pointer using target cursor, letting it room all over */
	status = XGrabPointer(dpy, root, False,
			      ButtonPressMask|ButtonReleaseMask, GrabModeSync,
			      GrabModeAsync, root, cursor, CurrentTime);
	if (status != GrabSuccess) fprintf(stderr, "Can't grab the mouse.");

	/* Let the user select a window... */
	while ((target_win == None) || (buttons != 0)) {
		/* allow one more event */
		XAllowEvents(dpy, SyncPointer, CurrentTime);
		XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
		switch (event.type) {
		case ButtonPress:
			if (target_win == None) {
				target_win = event.xbutton.subwindow; /* window selected */
				if (target_win == None)
					target_win = root;
			}
			buttons++;
			break;
		case ButtonRelease:
			if (buttons > 0) /* there may have been some down before we started */
				buttons--;
			break;
		}
	} 

	XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */

	return(target_win);
}
