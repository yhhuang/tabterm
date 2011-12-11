#include "appres.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "tabbar.h"

#undef WRESTLE_FOR_CHILD
#undef DEBUG
int init_stage = 0;

void
sighandler(int signum)
{
	if (signum == SIGCHLD)
		wait(0);
}

int set_signals()
{
        struct sigaction act;
        act.sa_handler = sighandler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_NOCLDSTOP | SA_NODEFER;
        sigaction(SIGCHLD, &act, 0);
        return 1;
}

char win_name[1024];
void
set_mainw_name(char * name)
{
	if (!gdi.display || !gdi.mainw)
		return;
	snprintf(win_name, 1024, "%s [tabterm]", name);
	XStoreName(gdi.display, gdi.mainw, win_name);
};

void
spawn_xterm()
{
	if (0 == fork()) {
		char buf[16];
		snprintf(buf, 15, "%d", (int)gdi.mainw);
		execlp("xterm", "xterm", "-into", buf, "-bw", "0", (char *)0);
		perror("Failed to create xterm window?");
		_exit(1);
	}
	return;
}


Window
create_main_window()
{
	XSetWindowAttributes winattr;
	winattr.background_pixel = WhitePixel(gdi.display, gdi.screen);
	gdi.mainw = XCreateWindow(gdi.display, gdi.root,
			0, 0, 10, 10, 0, CopyFromParent, CopyFromParent,
			gdi.visual, CWBackPixel, &winattr);

	if (!gdi.mainw) {
		perror("Oops, failed to create window");
		return 0;
	}

	XSelectInput(gdi.display, gdi.mainw,
		     0
		     |KeyPressMask
//		     |KeyReleaseMask
		     |ButtonPressMask
//		     |ButtonReleaseMask
//		     |EnterWindowMask
//		     |LeaveWindowMask
//		     |PointerMotionMask
//		     |PointerMotionHintMask
//		     |Button1MotionMask
//		     |Button2MotionMask
//		     |Button3MotionMask
//		     |Button4MotionMask
//		     |Button5MotionMask
//		     |ButtonMotionMask
//		     |KeymapStateMask
		     |ExposureMask
//		     |VisibilityChangeMask
		     |StructureNotifyMask
//		     |ResizeRedirectMask
		     |SubstructureNotifyMask
		     |SubstructureRedirectMask
		     |FocusChangeMask
		     |PropertyChangeMask
//		     |ColormapChangeMask
//		     |OwnerGrabButtonMask
		);

	set_mainw_name("[tabterm]");
#ifdef DEBUG
	printf("main window: %d(%08x)\n", (int)gdi.mainw, (int)gdi.mainw);
#endif
	return gdi.mainw;
}

void
match_size_hints(Window xterm, int add_y)
{
	XSizeHints * size_hints = XAllocSizeHints();
	long returned = 0;
	XGetWMNormalHints(gdi.display, xterm, size_hints, &returned);
	size_hints->base_height += add_y;
	size_hints->max_height += add_y;
	size_hints->min_height += add_y;
	size_hints->width_inc = 1;
	size_hints->height_inc = 1;
	XSetWMNormalHints(gdi.display, gdi.mainw, size_hints);
	XFree(size_hints);
}

extern Window get_parent(Window);
extern int is_expected_child(Window);
extern int received_child(Window);
extern int is_tabbed_window(Window w);

void
handle_reparent(XEvent * evt)
{
	XReparentEvent * xre = &(evt->xreparent);

	/* news about a child window, maybe:
	   it's our own window; (fine)
	   it's the first free xterm; (fine)
	   it's the wm giving it up; (we'll take it)
	   it's sawfish hesitating giving it up (we ask again)
	   it's we giving out a child
	*/
#ifdef WRESTLE_FOR_CHILD
	fprintf(stderr, "Reparent: e%08x w%08x p%08x\n",
		xre->event, xre->window, xre->parent);
	fprintf(stderr, "\tWe are: %08x\n", gdi.mainw);
#endif/*WRESTLE_FOR_CHILD*/

	if (xre->parent != gdi.mainw && is_tabbed_window(xre->window)) /* WM wants it back. No. */
	{
#ifdef WRESTLE_FOR_CHILD
		fprintf(stderr, "WM wants child back. No way.\n");
#endif/*WRESTLE_FOR_CHILD*/
		XReparentWindow(gdi.display, xre->window, gdi.mainw, 0, 0);
		return;
	}
	if (!is_expected_child(xre->window))	/* "don't care" category */
	    return;

	/* so we do expect this child, whose was it? */
	Window parent = get_parent(xre->window);
	if (parent == gdi.mainw) {
#ifdef WRESTLE_FOR_CHILD
		fprintf(stderr, "\tit's our child\n");
#endif/*WRESTLE_FOR_CHILD*/

		received_child(xre->window);
		XMoveResizeWindow(gdi.display, xre->window,
				  0, tab_options.tabs_on_top?the_bar.height:0,
				  gdi.width, gdi.height - the_bar.height);
//		match_size_hints(xre->window, the_bar.height);
		new_tab(xre->window);
	}
	else {
		/* One has to ask twice to take a child from sawfish.
		   So we do that.
		*/
#ifdef WRESTLE_FOR_CHILD
		fprintf(stderr, "\tit's NOT our child, i'll ask again\n");
#endif/*WRESTLE_FOR_CHILD*/
		XReparentWindow(gdi.display, xre->window, gdi.mainw, 0, 0);
	}
	return;
}

void
handle_maprequest(XEvent *evt)
{
	XMapRequestEvent * mre = &(evt->xmaprequest);
	XWindowAttributes attr;
	if (init_stage == 1) {
		init_stage = 2;
		XGetWindowAttributes(gdi.display, mre->window, &attr);

		init_tabbar(attr.width);
		the_bar.x=0; the_bar.y = attr.height;

		realize_tab_bar(gdi.mainw);

		XResizeWindow(gdi.display, gdi.mainw, attr.width,
			      attr.height + the_bar.height);
		match_size_hints(mre->window, the_bar.height);
		new_tab(mre->window);
	}
	else if(the_bar.win == mre->window) {
	}
	else {	/* it's our new child */
		XMoveResizeWindow(gdi.display, mre->window,
				  0, tab_options.tabs_on_top?the_bar.height:0,
				  gdi.width, gdi.height - the_bar.height);
		new_tab(mre->window);
	}
	XMapWindow(gdi.display, mre->window);
}

void
handle_destroy(XEvent * evt)
{
	/* this is where we know an xterm has quit by say, ctrl-D*/
	XDestroyWindowEvent * dwe = &(evt->xdestroywindow);
	if (delete_tab(dwe->window) == 0) exit(0); /* exit if no more tabs */
}

void
handle_configure_notify(XEvent * evt)
{
	XConfigureEvent * ce = &(evt->xconfigure);
	if (ce->window == gdi.mainw) {
		if ((ce->width != gdi.width ||
		     ce->height != gdi.height)
		    && the_bar.realized
			)
		{
			gdi.width = ce->width;
			gdi.height = ce->height;
			resize_term_windows(ce->width, gdi.height - the_bar.height);
		}
	}
}

/* when child xterm window wants to resize (e.g. due to font change)
 * we set it to our sub window size
 */
void
handle_configure_request(XEvent * evt) {
	XConfigureRequestEvent * cre = &(evt->xconfigurerequest);
	if (cre->window == gdi.mainw) /* it's us, do nothing */
		return;
#ifdef DEBUG
	fprintf(stderr, "ConfigureRequest: %d(%08x) %dx%d mask %ld\n"
		"resizing to %dx%d instead\n",
		(int)cre->window, (int)cre->window,
		cre->width, cre->height,
		cre->value_mask,
		gdi.width, gdi.height - the_bar.height);
#endif/*DEBUG*/

	XResizeWindow(gdi.display, cre->window,
			gdi.width, gdi.height - the_bar.height);

}

void
handle_key_press(XEvent * evt) {
	XKeyEvent * ke = &evt->xkey;
	if (ke->type == KeyPress || 1) {
#ifdef DEBUG
		fprintf(stderr, "KeyPress: %d state: %d\n",
			ke->keycode, ke->state);
#endif
	}
	int i;
	for (i = 0; i < MAX_KEYBINDS; i++) {
		if ((ke->keycode == key_bindings[i].keycode)
		    &&  ((ke->state&key_bindings[i].modifier) == key_bindings[i].modifier))
		{
			key_bindings[i].func();
			return;
		}
	}
	return;
}

#include "events.inc"

void
show_help(int argc, char * argv[])
{
	printf("Usage:\t%s [options]\n"
	       "\tAvailable options are:\n"
	       "\t-bg COLOR\ttab bar background\n"
	       "\t-tbg COLOR\tinactive tab background\n"
	       "\t-tfg COLOR\tinactive tab foreground\n"
	       "\t-abg COLOR\tactive tab background\n"
	       "\t-afg COLOR\tactive tab foreground\n"
	       "\t-bbg COLOR\tbutton background\n"
	       "\t-bfg COLOR\tbutton foreground\n"
	       "\t-fn FONT\tfont for tabs\n"
	       "\t-top\t\ttabs on top of terminal window\n"
	       "\t-bottom\t\ttabs at bottom of window (default)\n"
	       "\t-res\t\tshow default X resources\n"
	       "\t-h, -help\tshow this text\n",
	       argv[0]
		);
}

int first_spawn = 1;
int main(int argc, char * argv[])
{
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-help") == 0
		    || strcmp(argv[i], "-h") == 0)
		{
			show_help(argc, argv);
			return 0;
		}
		else if (strcmp(argv[i], "-res") == 0) {
			show_resources();
			return 0;
		}
	}

	init_appres(argc, argv);
	the_bar.realized = 0;

	if (!get_display_info() ||
	    !init_resources() ||
	    !create_main_window())
		exit (1);

	set_signals();
	XMapWindow(gdi.display, gdi.mainw);

	set_keybindings();

	XEvent evt;
	while (1) {
		XNextEvent(gdi.display, &evt);
#ifdef DEBUG
		printf("wid %08X\t%s\n", (int)(evt.xany.window),
		       event_names[evt.type]);
		fflush(stdout);
#endif

		switch(evt.type){
		case KeyPress:
			handle_key_press(&evt);
			break;
		case ReparentNotify:
			handle_reparent(&evt);
			break;
		case MapRequest:
			/* add a new tab */
			handle_maprequest(&evt);
			break;
		case DestroyNotify:
			/* remove a tab */
			handle_destroy(&evt);
			break;
		case ConfigureNotify:
			handle_configure_notify(&evt);
			break;
		case ConfigureRequest:
			handle_configure_request(&evt);
			break;
		case Expose:
			if (evt.xexpose.window != gdi.mainw)
				bar_handle_expose(&(evt.xexpose));
			break;
		case ButtonPress:
			bar_handle_button(&(evt.xbutton));
			break;
		case PropertyNotify:
			bar_handle_prop(&evt);
			break;
		case MapNotify:
			if (evt.xmap.window == gdi.mainw) {
				if (init_stage == 0) {
					init_stage ++;
					spawn_xterm();
				}
			}
			break;
		case UnmapNotify:
			if (the_bar.tab_group.tab_count == 0)
				exit(0);
			break;
		case EnterNotify:
			set_focus();
			break;
		default:
			break;
		}
		XFlush(gdi.display);
	};
	return 0;
}
