#include "button.h"
#include "tabbar.h"
#include "appres.h"

#include <stdlib.h>
#include <string.h>

button_options_t button_params;

void set_button_params()
{
	button_params.margin_top = 0;
	button_params.margin_bottom = 0;
	button_params.padding_bottom = 1;
	button_params.padding_top = 1;
	button_params.padding_side_percent = 50;
	button_params.spacing = 1;
	XFontStruct* font = tab_options.font;
	int char_height = font->ascent + font->descent;
	button_params.height = char_height
		+ button_params.padding_bottom
		+ button_params.padding_top -1;
	button_params.width = font->max_bounds.width *
		(1 + (button_params.padding_side_percent * 2.0)/100);
}

pw_button *
create_button(char label, void (*click_func)())
{
	pw_button * btn = (pw_button *)malloc(sizeof(pw_button));
	if (!btn)
		return 0;
//	XGCValues values;
//	Window w = XCreateSimpleWindow(dpy, parent, x, y, width, height,
//				       0, 0, tab_options.btn_bg);
	btn->width  = button_params.width;
	btn->height = button_params.height;
	btn->state  = 0; /* not used yet */
	btn->label  = label;
	btn->on_click = click_func;
//	btn->gc = XCreateGC(dpy, w, 0, &values);
//	XSelectInput(dpy, w, ButtonPressMask | ExposureMask);
//	XMapWindow(dpy, w);
	return btn;
};

pw_button * find_button_by_wid(Window w, pw_button_group * pbg)
{
	int i;
	for (i = 0; i < pbg->num_buttons; i ++) {
		if (pbg->buttons[i]->win == w)
			return pbg->buttons[i];
	}
	return 0;
}

int paint_button(pw_button * btn)
{
	Display * dpy = gdi.display;
	XFontStruct * font = tab_options.font;
	XSetForeground(dpy, btn->gc, tab_options.btn_border);
	XDrawRectangle(dpy, btn->win, btn->gc, 0, 0, btn->width -1, btn->height-1);
	XSetForeground(dpy, btn->gc, tab_options.btn_fg);
	XSetFont(dpy, btn->gc, font->fid);
	XDrawString(dpy, btn->win, btn->gc, btn->text_x, btn->text_y, &(btn->label), 1);
	return 1;
}

int process_button_event(pw_button * btn, XButtonEvent * evt)
{
	if (!btn)
		return 0;
	if (btn->on_click)
		btn->on_click();
	else {
		fprintf(stderr, "on_click undefined\n");
	}
	return 1;
}

int move_button_group(pw_button_group * pbg, int x)
{
	pbg->x = x;
	XMoveWindow(gdi.display, pbg->win, x, pbg->y);
	return 1;
}

void init_button_group(pw_button_group * pbg)
{
	pbg->x = 0;
	pbg->y = 0;
	pbg->width = 0;
	pbg->height = 0;
	pbg->num_buttons = 0;
	set_button_params();
}

int
add_button_to_group(pw_button_group * pbg, pw_button * btn)
{
	if (pbg->num_buttons >= MAX_BUTTONS)
		return 0;
	btn->x = pbg->num_buttons * button_params.width
		+ (pbg->num_buttons + 1) * button_params.spacing;

	btn->y = button_params.margin_top;
	
	btn->width = button_params.width;
	btn->height = button_params.height;
	btn->text_y = tab_options.font->ascent + button_params.padding_top;

	btn->text_x = tab_options.font->max_bounds.width *
		(float)(button_params.padding_side_percent) / 100;

	pbg->buttons[pbg->num_buttons] = btn;
	pbg->num_buttons ++;
	
	pbg->width = pbg->num_buttons * button_params.width
		+ (pbg->num_buttons + 1) * button_params.spacing;
	pbg->height = button_params.height;

	return 1;
}

int
realize_button_group(pw_button_group * pbg, Window parent)
{
	XGCValues values;
	Display * dpy = gdi.display;
	int i;
	pbg->win = XCreateSimpleWindow(dpy, parent,
				       pbg->x, pbg->y,
				       pbg->width,
				       pbg->height,
				       0, 0, tab_options.bg);

	for (i = 0; i < pbg->num_buttons; i++) {
		pw_button * btn = pbg->buttons[i];
		btn->win = XCreateSimpleWindow(dpy, pbg->win,
					       btn->x, btn->y,
					       btn->width,
					       btn->height,
					       0, 0, tab_options.btn_bg);
		btn->gc = XCreateGC(dpy, btn->win, 0, &values);
		XSelectInput(dpy, btn->win, ButtonPressMask | ExposureMask);
		XMapWindow(dpy, btn->win);
	}
	XMapWindow(dpy, pbg->win);
	return 1;
}

