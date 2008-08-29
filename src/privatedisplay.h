#ifndef _PRIVATEDISPLAY_H
#define _PRIVATEDISPLAY_H

#include <X11/XKBlib.h>

#include <compiz-core.h>
#include <compdisplay.h>
#include <compmetadata.h>

#define COMP_DISPLAY_OPTION_ACTIVE_PLUGINS                   0
#define COMP_DISPLAY_OPTION_CLICK_TO_FOCUS                   1
#define COMP_DISPLAY_OPTION_AUTORAISE                        2
#define COMP_DISPLAY_OPTION_AUTORAISE_DELAY                  3
#define COMP_DISPLAY_OPTION_CLOSE_WINDOW_KEY                 4
#define COMP_DISPLAY_OPTION_CLOSE_WINDOW_BUTTON              5
#define COMP_DISPLAY_OPTION_MAIN_MENU_KEY                    6
#define COMP_DISPLAY_OPTION_RUN_DIALOG_KEY                   7
#define COMP_DISPLAY_OPTION_COMMAND0                         8
#define COMP_DISPLAY_OPTION_COMMAND1                         9
#define COMP_DISPLAY_OPTION_COMMAND2                         10
#define COMP_DISPLAY_OPTION_COMMAND3                         11
#define COMP_DISPLAY_OPTION_COMMAND4                         12
#define COMP_DISPLAY_OPTION_COMMAND5                         13
#define COMP_DISPLAY_OPTION_COMMAND6                         14
#define COMP_DISPLAY_OPTION_COMMAND7                         15
#define COMP_DISPLAY_OPTION_COMMAND8                         16
#define COMP_DISPLAY_OPTION_COMMAND9                         17
#define COMP_DISPLAY_OPTION_COMMAND10                        18
#define COMP_DISPLAY_OPTION_COMMAND11                        19
#define COMP_DISPLAY_OPTION_RUN_COMMAND0_KEY                 20
#define COMP_DISPLAY_OPTION_RUN_COMMAND1_KEY                 21
#define COMP_DISPLAY_OPTION_RUN_COMMAND2_KEY                 22
#define COMP_DISPLAY_OPTION_RUN_COMMAND3_KEY                 23
#define COMP_DISPLAY_OPTION_RUN_COMMAND4_KEY                 24
#define COMP_DISPLAY_OPTION_RUN_COMMAND5_KEY                 25
#define COMP_DISPLAY_OPTION_RUN_COMMAND6_KEY                 26
#define COMP_DISPLAY_OPTION_RUN_COMMAND7_KEY                 27
#define COMP_DISPLAY_OPTION_RUN_COMMAND8_KEY                 28
#define COMP_DISPLAY_OPTION_RUN_COMMAND9_KEY                 29
#define COMP_DISPLAY_OPTION_RUN_COMMAND10_KEY                30
#define COMP_DISPLAY_OPTION_RUN_COMMAND11_KEY                31
#define COMP_DISPLAY_OPTION_RAISE_WINDOW_KEY                 32
#define COMP_DISPLAY_OPTION_RAISE_WINDOW_BUTTON              33
#define COMP_DISPLAY_OPTION_LOWER_WINDOW_KEY                 34
#define COMP_DISPLAY_OPTION_LOWER_WINDOW_BUTTON              35
#define COMP_DISPLAY_OPTION_UNMAXIMIZE_WINDOW_KEY            36
#define COMP_DISPLAY_OPTION_MINIMIZE_WINDOW_KEY              37
#define COMP_DISPLAY_OPTION_MINIMIZE_WINDOW_BUTTON           38
#define COMP_DISPLAY_OPTION_MAXIMIZE_WINDOW_KEY              39
#define COMP_DISPLAY_OPTION_MAXIMIZE_WINDOW_HORZ_KEY         40
#define COMP_DISPLAY_OPTION_MAXIMIZE_WINDOW_VERT_KEY         41
#define COMP_DISPLAY_OPTION_SCREENSHOT                       42
#define COMP_DISPLAY_OPTION_RUN_SCREENSHOT_KEY               43
#define COMP_DISPLAY_OPTION_WINDOW_SCREENSHOT                44
#define COMP_DISPLAY_OPTION_RUN_WINDOW_SCREENSHOT_KEY        45
#define COMP_DISPLAY_OPTION_WINDOW_MENU_BUTTON               46
#define COMP_DISPLAY_OPTION_WINDOW_MENU_KEY                  47
#define COMP_DISPLAY_OPTION_SHOW_DESKTOP_KEY                 48
#define COMP_DISPLAY_OPTION_SHOW_DESKTOP_EDGE                49
#define COMP_DISPLAY_OPTION_RAISE_ON_CLICK                   50
#define COMP_DISPLAY_OPTION_AUDIBLE_BELL                     51
#define COMP_DISPLAY_OPTION_TOGGLE_WINDOW_MAXIMIZED_KEY      52
#define COMP_DISPLAY_OPTION_TOGGLE_WINDOW_MAXIMIZED_BUTTON   53
#define COMP_DISPLAY_OPTION_TOGGLE_WINDOW_MAXIMIZED_HORZ_KEY 54
#define COMP_DISPLAY_OPTION_TOGGLE_WINDOW_MAXIMIZED_VERT_KEY 55
#define COMP_DISPLAY_OPTION_HIDE_SKIP_TASKBAR_WINDOWS        56
#define COMP_DISPLAY_OPTION_TOGGLE_WINDOW_SHADED_KEY         57
#define COMP_DISPLAY_OPTION_IGNORE_HINTS_WHEN_MAXIMIZED      58
#define COMP_DISPLAY_OPTION_TERMINAL			     59
#define COMP_DISPLAY_OPTION_RUN_TERMINAL_KEY		     60
#define COMP_DISPLAY_OPTION_PING_DELAY			     61
#define COMP_DISPLAY_OPTION_EDGE_DELAY                       62
#define COMP_DISPLAY_OPTION_NUM				     63

extern const CompMetadata::OptionInfo
coreDisplayOptionInfo[COMP_DISPLAY_OPTION_NUM];

extern bool inHandleEvent;

extern CompScreen *targetScreen;
extern CompOutput *targetOutput;

typedef struct _CompDelayedEdgeSettings
{
    CompAction::CallBack initiate;
    CompAction::CallBack terminate;

    unsigned int edge;
    unsigned int state;

    CompOption::Vector options;
} CompDelayedEdgeSettings;


class PrivateDisplay {

    public:
	PrivateDisplay (CompDisplay *display);
	~PrivateDisplay ();
	
	void
	updatePlugins ();

	bool
	triggerButtonPressBindings (CompOption::Vector &options,
				    XEvent             *event,
				    CompOption::Vector &arguments);

	bool
	triggerButtonReleaseBindings (CompOption::Vector &options,
				      XEvent             *event,
				      CompOption::Vector &arguments);

	bool
	triggerKeyPressBindings (CompOption::Vector &options,
				 XEvent             *event,
				 CompOption::Vector &arguments);

	bool
	triggerKeyReleaseBindings (CompOption::Vector &options,
				   XEvent             *event,
				   CompOption::Vector &arguments);

	bool
	triggerStateNotifyBindings (CompOption::Vector  &options,
				    XkbStateNotifyEvent *event,
				    CompOption::Vector  &arguments);
	bool
	triggerEdgeEnter (unsigned int       edge,
			  CompAction::State  state,
     			  CompOption::Vector &arguments);

	void
	setAudibleBell (bool audible);

	bool
	handlePingTimeout ();
	
	bool
	handleActionEvent (XEvent *event);

	void
	handleSelectionRequest (XEvent *event);

	void
	handleSelectionClear (XEvent *event);

	void
	initAtoms ();

    public:

	CompDisplay *display;

	xcb_connection_t *connection;

	Display    *dpy;
	CompScreenList screens;

	CompWatchFdHandle watchFdHandle;

	int syncEvent, syncError;

	bool randrExtension;
	int  randrEvent, randrError;

	bool shapeExtension;
	int  shapeEvent, shapeError;

	bool xkbExtension;
	int  xkbEvent, xkbError;

	bool xineramaExtension;
	int  xineramaEvent, xineramaError;

	std::vector<XineramaScreenInfo> screenInfo;

	SnDisplay *snDisplay;

	unsigned int      lastPing;
	CompCore::Timer   pingTimer;

	Window activeWindow;

	Window below;
	char   displayString[256];

	XModifierKeymap *modMap;
	unsigned int    modMask[CompModNum];
	unsigned int    ignoredModMask;

	KeyCode escapeKeyCode;
	KeyCode returnKeyCode;

	CompOption::Vector opt;

	CompCore::Timer autoRaiseTimer;
	Window	      autoRaiseWindow;

	CompCore::Timer edgeDelayTimer;
	CompDelayedEdgeSettings edgeDelaySettings;

	CompOption::Value plugin;
	bool	          dirtyPluginList;

	CompDisplay::Atoms atoms;

};

#endif