#ifndef BUTTON_H
#define BUTTON_H
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#define BTN_ENABLE	1
#define BTN_ACTIVE	2
#define BTN_VISIBLE	4

typedef struct{
	Window win;
	char label;
	int x, y, width, height;
	int text_x, text_y;
	int state;
	GC gc;
	void (*on_click)();
}pw_button;

#define MAX_BUTTONS 32
typedef struct {
	Window win;
	int x, y, width, height;
	int num_buttons;
	pw_button * buttons[MAX_BUTTONS];
}pw_button_group;

typedef struct {
	int height, width;
	int margin_top, margin_bottom, spacing;
	int padding_top, padding_bottom, padding_side_percent;
}button_options_t;

extern button_options_t button_params;
void set_button_params();

pw_button *
create_button(char label, void(*click_func)());

int move_button_group(pw_button_group * btn, int x);
int process_button_event(pw_button * btn, XButtonEvent * evt);
pw_button * find_button_by_wid(Window w, pw_button_group * pbg);
int paint_button(pw_button* btn);

int add_button_to_group(pw_button_group * pbg, pw_button * btn);
void init_button_group(pw_button_group * pbg);
int realize_button_group(pw_button_group * pbg, Window parent);

#endif/*BUTTON_H*/
