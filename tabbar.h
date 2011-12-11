#ifndef TABBAR_H
#define TABBAR_H

#include "tabs.h"
#include "button.h"

typedef struct{
	int realized;
	Window win;
	int x, y, width, height;
	pw_tab_group tab_group;
	pw_button_group left_btns, right_btns;
}tabbar_t;

extern tabbar_t the_bar;

int init_tabbar(int width); /* height is auto */
int realize_tab_bar(Window parent);
int set_bar_geometry(tabbar_t * tb, int x, int y, int width);
Window get_active_term(tabbar_t * tb);
int new_tab(Window w);
int delete_tab(Window w);
int bar_handle_expose(XExposeEvent * ee);
int bar_handle_button(XButtonEvent * xbtn);
int bar_handle_prop(XEvent * evt);
int resize_term_windows(int width, int height);
void set_focus();
void shift_bar_left();
void shift_bar_right();
void kill_active_tab();
void swallow_window();
void next_tab();
void prev_tab();
void release_window();
Window Select_Window();

#endif/*TABBAR_H*/
