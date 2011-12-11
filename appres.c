#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "appres.h"

global_display_info gdi;
AppResourcesRec app_res;
tab_options_t tab_options;

static XtResource appResourcesSpec[] = {
        {"background", "Background", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, background),
         XtRString, (XtPointer) "white"},

        {"tabforeground", "tabForeground", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, tabfg),
         XtRString, (XtPointer) "black"},

        {"tabbackground", "tabBackground", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, tabbg),
         XtRString, (XtPointer) "white"},

        {"tabborder", "tabBorder", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, tabborder),
         XtRString, (XtPointer) "white"},

        {"activeforeground", "activeForeground", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, activefg),
         XtRString, (XtPointer) "white"},

        {"activebackground", "activeBackground", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, activebg),
         XtRString, (XtPointer) "black"},

        {"activeborder", "activeBorder", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, activeborder),
         XtRString, (XtPointer) "white"},

        {"btnforeground", "btnForeground", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, btnfg),
         XtRString, (XtPointer) "white"},

        {"btnbackground", "btnBackground", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, btnbg),
         XtRString, (XtPointer) "black"},

        {"btnborder", "btnBorder", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, btnborder),
         XtRString, (XtPointer) "white"},

        {"fontName", "FontName", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, fontName),
         XtRString, (XtPointer) "fixed"},

        {"topTabs", "TopTabs", XtRBoolean, sizeof (Boolean),
         XtOffsetOf (AppResourcesRec, tabontop),
         XtRBoolean, (XtPointer) False},

        {"keyNew", "KeyNew", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keynew),
         XtRString, (XtPointer) "Shift Ctrl N"},

        {"keyLeft", "KeyLeft", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keyleft),
         XtRString, (XtPointer) "Shift Ctrl Right"},

        {"keyRight", "KeyRight", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keyright),
         XtRString, (XtPointer) "Shift Ctrl Left"},

        {"keyAttach", "KeyAttach", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keyattach),
         XtRString, (XtPointer) "Shift Ctrl A"},

        {"keyDetach", "KeyDetach", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keydetach),
         XtRString, (XtPointer) "Shift Ctrl D"},

        {"keyCycle", "KeyCycle", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keynext),
         XtRString, (XtPointer) "Ctrl Tab"},

        {"keyCycleBack", "KeyCycleBack", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keyprev),
         XtRString, (XtPointer) "Ctrl Shift Tab"},

        {"keyKill", "KeyKill", XtRString, sizeof (String),
         XtOffsetOf (AppResourcesRec, keykill),
         XtRString, (XtPointer) "Shift Ctrl K"}
};

void show_resources()
{
	printf("TabTerm*Background: white\n"
	       "TabTerm*tabForeground: black\n"
	       "TabTerm*tabBackground: white\n"
	       "TabTerm*tabBorder: white\n"
	       "TabTerm*activeForeground: white\n"
	       "TabTerm*activeBackground: black\n"
	       "TabTerm*activeBorder: white\n"
	       "TabTerm*btnForeground: white\n"
	       "TabTerm*btnBackground: black\n"
	       "TabTerm*btnBorder: white\n"
	       "TabTerm*FontName: fixed\n"
	       "TabTerm*TopTabs: TopTabs\n"
	       "TabTerm*KeyNew: Shift Ctrl N\n"
	       "TabTerm*KeyLeft: Shift Ctrl Right\n"
	       "TabTerm*KeyRight: Shift Ctrl Left\n"
	       "TabTerm*KeyAttach: Shift Ctrl A\n"
	       "TabTerm*KeyDetach: Shift Ctrl D\n"
	       "TabTerm*KeyCycle: Ctrl Tab\n"
	       "TabTerm*KeyCycleBack: Ctrl Shift Tab\n"
	       "TabTerm*KeyKill: Shift Ctrl K\n");
};

static XrmOptionDescRec optionsSpec[] = {
        {"-bg",	    "*background",       XrmoptionSepArg, 0},
        {"-tbg",    "*tabbackground",    XrmoptionSepArg, 0},
        {"-tfg",    "*tabforeground",    XrmoptionSepArg, 0},
        {"-tbd",    "*tabborder",        XrmoptionSepArg, 0},
        {"-abg",    "*activebackground", XrmoptionSepArg, 0},
        {"-afg",    "*activeforeground", XrmoptionSepArg, 0},
        {"-abd",    "*activeborder",     XrmoptionSepArg, 0},
	{"-bbg",    "*btnbackground",    XrmoptionSepArg, 0},
        {"-bfg",    "*btnforeground",    XrmoptionSepArg, 0},
        {"-bbd",    "*btnborder",        XrmoptionSepArg, 0},
	{"-fn",	    "*fontName",         XrmoptionSepArg, 0},
	{"-top",    "*TopTabs",          XrmoptionNoArg,  "True"},
	{"-bottom", "*TopTabs",          XrmoptionNoArg,  "False"}
};

AppResourcesRec app_res;

int init_appres(int argc, char * argv[]) {
        XtAppContext app;
        Widget wTop;
	/* initialize Xt, including command line options */
        wTop = XtAppInitialize (&app, "TabTerm",
                                optionsSpec, XtNumber (optionsSpec),
                                &argc, argv, NULL, NULL, 0);

	/* load application resources */
        XtGetApplicationResources (wTop, (XtPointer) & app_res,
                                   appResourcesSpec,
                                   XtNumber (appResourcesSpec), NULL, 0);
	return 1;
}

int
get_display_info()
{
	char * ed = getenv("DISPLAY");
	if (!ed) {
		perror("Display undefined\n");
		return 0;
	}

	if (!(gdi.display = XOpenDisplay(ed))) {
		perror("Failed to connect to display");
		return 0;
	}

	gdi.root = DefaultRootWindow(gdi.display);
	gdi.screen = DefaultScreen(gdi.display);
	gdi.depth = DefaultDepth(gdi.display, gdi.screen);
	gdi.visual = DefaultVisual(gdi.display, gdi.screen);
	gdi.cmap = DefaultColormap(gdi.display, gdi.screen);
	return 1;
}

int
init_resources()
{
char * const defaultFont = "fixed";

redo_font:
	tab_options.font = XLoadQueryFont(gdi.display, app_res.fontName);
	if (!tab_options.font) {
		if (strcmp(app_res.fontName, defaultFont)) {
			fprintf(stderr, "Failed to use font %s; trying `%s'\n",
				app_res.fontName, defaultFont);
			app_res.fontName = defaultFont;
			goto redo_font;
		}
		else {
			fprintf(stderr, "Failed to use font `%s'; I give up.\n",
				app_res.fontName);
			return 0;
		}
	}
	alloc_colors();
	tab_options.tabs_on_top = app_res.tabontop;
	
	return 1;
}

int
alloc_colors() {
	unsigned long black = BlackPixel(gdi.display, gdi.screen);
	unsigned long white = WhitePixel(gdi.display, gdi.screen);

	tab_options.bg = get_color(app_res.background, white);

	tab_options.tab_fg = get_color(app_res.tabfg, black);
	tab_options.tab_bg = get_color(app_res.tabbg, white);
	tab_options.tab_border = get_color(app_res.tabborder, white);

	tab_options.active_fg = get_color(app_res.activefg, white);
	tab_options.active_bg = get_color(app_res.activebg, black);
	tab_options.active_border = get_color(app_res.activeborder, white);

	tab_options.btn_fg = get_color(app_res.btnfg, white);
	tab_options.btn_bg = get_color(app_res.btnbg, black);
	tab_options.btn_border = get_color(app_res.btnborder, white);

	return 1;
};

unsigned long
get_color(char * colorname, unsigned long pixel_fallback)
{
	XColor exact_def;
	int i = 5;
	XVisualInfo visual_info;

	/* Try to allocate colors for PseudoColor, TrueColor,
	 * DirectColor, and StaticColor; use black and white
	 * for StaticGray and GrayScale */
	if (gdi.depth == 1) {
		/* Must be StaticGray, use black and white */
		goto color_fallback;
	}
	while (!XMatchVisualInfo(gdi.display, gdi.screen, gdi.depth,
				 /* visual class */i--, &visual_info) && i>=0);

	if (i < StaticColor) { /* Color visual classes are 2 to 5 */
		/* No color visual available at default depth;
		 * some applications might call XMatchVisualInfo
		 * here to try for a GrayScale visual if they
		 * can use gray to advantage, before giving up
		 * and using black and white */
		goto color_fallback;
	}

	if (!XParseColor (gdi.display, gdi.cmap, colorname,
			  &exact_def))
		goto color_fallback;

	if (!XAllocColor(gdi.display, gdi.cmap, &exact_def)) {
		fprintf(stderr, "can't allocate color:\n");
		fprintf(stderr, "All colorcells allocated and\n");
		fprintf(stderr, "no matching cell found.\n");
		goto color_fallback;
	}
	return exact_def.pixel;

color_fallback:
	fprintf(stderr, "Can't allocate color %s, using fallback\n",
		colorname);
	return pixel_fallback;
}

int parse_key_binding(char * string, KeyCode * keycode, unsigned int * modifier)
{
	int begin = 0, length = 0;
	int total_len = strlen(string);
	int find_next_string()
	{
		begin += length;
		length = 0;
		for (; begin < total_len; begin++) { /* skip white spaces */
			if (! isspace(string[begin]))
				break;
		}
		for (length = 0; length + begin < total_len; length ++) {
			if (isspace(string[begin + length]))
			    break;
		}
		return length;
	};
	KeySym sym;
	* keycode = 0;
	* modifier = 0;
	char token[128];
	while(find_next_string()) {
		if (length > 127) /* too long, must be wrong */
			goto key_parse_fail;
		strncpy(token, string+begin, length);
		token[length] = '\0';

//		fprintf(stderr, "Token is: %s\n", token);

		sym = XStringToKeysym(token);
		if (sym == NoSymbol) { /* maybe it's a modifier */
			if (!strcmp(token, "Shift")) {
				*modifier |= ShiftMask;
				continue;
			}else if (!strcmp(token, "Ctrl")) {
				*modifier |= ControlMask;
				continue;
			}else if (!strcmp(token, "Mod1")) {
				*modifier |= Mod1Mask;
				continue;
			}else if (!strcmp(token, "Mod2")) {
				*modifier |= Mod2Mask;
				continue;
			}else if (!strcmp(token, "Mod3")) {
				*modifier |= Mod3Mask;
				continue;
			}else if (!strcmp(token, "Mod4")) {
				*modifier |= Mod4Mask;
				continue;
			}else if (!strcmp(token, "Mod5")) {
				*modifier |= Mod5Mask;
				continue;
			}else
				goto key_parse_fail;
		}
		else{
			if (*keycode != 0)  /* already assigned */
				goto key_parse_fail;
			*keycode = XKeysymToKeycode(gdi.display, sym);
		}
	}
	return 1;
key_parse_fail:	
	fprintf(stderr, "Failed to parse keybinding: %s\n", string);
				return 0;
}

void set_keybindings() {
	assign_binding(key_bindings, app_res.keynew);
	assign_binding(key_bindings+1, app_res.keyleft);
	assign_binding(key_bindings+2, app_res.keyright);
	assign_binding(key_bindings+3, app_res.keyattach);
	assign_binding(key_bindings+4, app_res.keydetach);
	assign_binding(key_bindings+5, app_res.keykill);
	assign_binding(key_bindings+6, app_res.keynext);
	assign_binding(key_bindings+7, app_res.keyprev);
}

int assign_binding(key_binding_t * kbt, String string) {
	if (!parse_key_binding(string, &(kbt->keycode), &(kbt->modifier))) {
		fprintf(stderr, "failed to bind keys for %s\n", kbt->name);
		return 0;
	}
	XGrabKey(gdi.display, kbt->keycode, kbt->modifier,
		 gdi.mainw, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(gdi.display, kbt->keycode, kbt->modifier|Mod2Mask,
		 gdi.mainw, False, GrabModeAsync, GrabModeAsync);
	return 1;
}
