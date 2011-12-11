#ifndef TABS_H
#define TABS_H
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define TAB_LABEL_LEN 64
#define MAX_TABS 128
#define TAB_ACTIVE 1

typedef struct {
	char label[TAB_LABEL_LEN];
	Window w; /* the win id of an xterm window */
}tab_t;

typedef struct {
	Window win;
	tab_t tabs[MAX_TABS];
	int tab_count;
	int x, y, width, height;
	int scroll_offset;
	int tab_width;
	int label_x_offset, label_y_offset, label_text_len;
	GC gc;
	XPoint node[5];
	int active;
}pw_tab_group;

int realize_tab_group(Window parent);
int init_tab_group(int x, int y, int width, int height);
int move_tab_group(int x, int y);
int resize_tab_group(int w, int h);
int add_tab(Window w);
int remove_tab(Window w);
int draw_tabs(int force);
int set_active_tab(int idx);
int tab_click(int x, int y);
int find_tab_by_wid(Window w);
int tab_set_name(int idx, char * name);

#endif/*TABS_H*/
