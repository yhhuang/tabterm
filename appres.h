#ifndef Y_APPRES
#define Y_APPRES

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <stdio.h>

typedef struct
{
	char * name;
	KeyCode keycode;
	unsigned int modifier;
	void (*func)();
}key_binding_t;

#define MAX_KEYBINDS 8

extern key_binding_t key_bindings[MAX_KEYBINDS];

typedef struct
{
	String background;
	String tabfg, tabbg, tabborder;
	String activefg, activebg, activeborder;
	String btnfg, btnbg, btnborder;
	String fontName;
	Boolean tabontop;
	String keynew, keyleft, keyright, keyattach, keydetach, keykill,
		keynext, keyprev;
} AppResourcesRec;

typedef struct
{
	XFontStruct * font;
	unsigned long int bg,
		tab_fg, tab_bg, tab_border,
		active_fg, active_bg, active_border,
		btn_fg, btn_bg, btn_border;
	int tabs_on_top;
}tab_options_t;

typedef struct{
        Display * display;
        int screen;
        unsigned depth;
        Visual * visual;
	Colormap cmap;
        Window root, mainw;
        int width, height;
}global_display_info;

extern global_display_info gdi;
extern AppResourcesRec app_res;
extern tab_options_t tab_options;

/* get_display_info:
   get information about the display
*/
int get_display_info();

/* init_appres:
   parses X resources and commandline options
*/
int init_appres(int argc, char * argv[]);

/* init_resources:
   setup fonts, allocate colors, etc.
*/
int init_resources();
unsigned long
get_color(char * colorname, unsigned long pixel_fallback);

void set_keybindings();
int alloc_colors();
int assign_binding(key_binding_t * kbt, char * string);
void show_resources();
#endif/*Y_APPRES*/
