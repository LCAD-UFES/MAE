/*
 *  forms.h   $Revision: 1.1.1.1 $ $State: Exp $
 *            $Date: 2005/01/15 22:02:06 $ $Author: cvs $
 *
 *. All XForms files as distributed in this package are
 *  Copyright(c) 1996-1998 by T.C. Zhao and Mark Overmars.
 *  ALL RIGHTS RESERVED.
 *
 * Permission to use, copy, and distribute this software in its entirety
 * for non-commercial purposes and without fee, is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies and their documentation.
 *
 * If you intend to use xforms commercially, this includes in-house
 * use and consulting, you must contact the authors at
 * xforms@world.std.com for a license arrangement. Running a publicly
 * available freeware that requires xforms is not considered commercial use.
 *
 * This software is provided "as is" without expressed or implied
 * warranty of any kind.
 *
 * You may not "bundle" and distribute this software with commercial
 * systems and/or other distribution media without prior consent of the
 * authors.  The only exception is for Linux CD distribution of
 * free software that requires xforms and in that case, xforms
 * can be bundled and repackaged.
 *
 * Contact zhao@bloch.phys.uwm.edu or visit
 * http://bloch.phys.uwm.edu/xforms  or
 * http://bragg.phys.uwm.edu/xforms 
 * if you have questions about xforms.
 *
 * ******** This file is generated automatically. DO NOT CHANGE *********
 */

#ifndef FL_FORMS_H
#define FL_FORMS_H

#define FL_VERSION             0
#define FL_REVISION           88
#define FL_FIXLEVEL            1
#define FL_INCLUDE_VERSION    (FL_VERSION * 1000 + FL_REVISION)

#include <stdio.h>
#include <string.h>
#include <limits.h>

#if defined(__cplusplus)
extern "C"
{
#endif

#if !defined(FL_WIN32) || !defined(SHARED_LIB)
#define FL_EXPORT extern
#else
#ifdef MAKING_FORMS
#define FL_EXPORT __declspec( dllexport ) extern
#else
#define FL_EXPORT __declspec( dllimport ) extern
#endif				/* MAKING_FORMS */
#endif				/* FL_WIN32 */

/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *  Basic definitions and limits.
 *  Window system independent prototypes
 *
 *  Modify with care
 *
 */

#ifndef FL_BASIC_H
#define FL_BASIC_H		/* { */

/* for compatibilities */
#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

/* some general constants */
enum
{
    FL_ON = 1,
    FL_OK = 1,
    FL_VALID = 1,
    FL_PREEMPT = 1,
    FL_AUTO = 2,
    FL_WHEN_NEEDED = FL_AUTO,

    FL_OFF = 0,
    FL_CANCEL = 0,
    FL_INVALID = 0,

    /* FL_NONE = 0,  defined elsewhere */

    /* WM_DELETE_WINDOW callback return */
    FL_IGNORE = -1,
    FL_CLOSE = -2
};

/* Max  directory length  */

#ifndef FL_PATH_MAX
#ifndef PATH_MAX
#define FL_PATH_MAX       1024
#else
#define FL_PATH_MAX       PATH_MAX
#endif
#endif /* !def FL_PATH_MAX */

/*
 * The screen coordinate unit, FL_Coord, must be of signed type.
 * If FL_Coord is float, FL_CoordIsFloat must be defined to be 1 so that
 * round-off error can be checked. **TODO Float not tested ***
 */

typedef int FL_Coord;
#define FL_COORD        FL_Coord
#define FL_CoordIsFloat 0	/* make it 1 if FL_Coord is of type float */

typedef unsigned long FL_COLOR;

/*
 * Coordinates can be in pixels, milli-meters or points (1/72inch)
 */
typedef enum
{
    FL_COORD_PIXEL,		/* default, Pixel           */
    FL_COORD_MM,		/* milli-meter              */
    FL_COORD_POINT,		/* point                    */
    FL_COORD_centiMM,		/* one hundredth of a mm    */
    FL_COORD_centiPOINT		/* one hundredth of a point */
}
FL_COORD_UNIT;

/*
 * All object classes.
 */

typedef enum
{
    FL_INVALID_CLASS,
    FL_BUTTON, FL_LIGHTBUTTON,
    FL_ROUNDBUTTON, FL_ROUND3DBUTTON,
    FL_CHECKBUTTON,
    FL_BITMAPBUTTON, FL_PIXMAPBUTTON,
    FL_BITMAP, FL_PIXMAP,
    FL_BOX, FL_TEXT,
    FL_MENU, FL_CHART, FL_CHOICE,
    FL_COUNTER, FL_SLIDER, FL_VALSLIDER, FL_INPUT,
    FL_BROWSER,
    FL_DIAL,
    FL_TIMER,
    FL_CLOCK,
    FL_POSITIONER,
    FL_FREE,
    FL_XYPLOT,
    FL_FRAME,
    FL_LABELFRAME,
    FL_CANVAS,
    FL_GLCANVAS,
    FL_TABFOLDER,
    FL_SCROLLBAR,
    FL_SCROLLBUTTON,
    FL_MENUBAR,
    FL_IMAGECANVAS,
    FL_TEXTBOX,			/* for internal use     */
    FL_FLATBUTTON
}
FL_CLASS;

#define FL_BEGIN_GROUP    10000
#define FL_END_GROUP      20000

#define FL_USER_CLASS_START   1001	/* min. user class  value */
#define FL_USER_CLASS_END     9999	/* max. user class  value */

/* how to display a form onto screen */

typedef enum
{
    FL_PLACE_FREE = 0,		/* size remain resizable      */
    FL_PLACE_MOUSE = 1,		/* mouse centered on form     */
    FL_PLACE_CENTER = 2,	/* center of the screen       */
    FL_PLACE_POSITION = 4,	/* specific size              */
    FL_PLACE_SIZE = 8,		/* specific size              */
    FL_PLACE_GEOMETRY = 16,	/* specific position          */
    FL_PLACE_ASPECT = 32,	/* keep aspect ratio          */
    FL_PLACE_FULLSCREEN = 64,	/* scale to fit screen        */
    FL_PLACE_HOTSPOT = 128,	/* so mouse fall on (x,y)     */
    FL_PLACE_ICONIC = 256,
    /* modifier */
    FL_FREE_SIZE = (1 << 14),
    FL_FIX_SIZE = (1 << 15)
}
FL_PLACE;

#define FL_PLACE_FREE_CENTER (FL_PLACE_CENTER|FL_FREE_SIZE)
#define FL_PLACE_CENTERFREE  (FL_PLACE_CENTER|FL_FREE_SIZE)

/* Window manager decoration request and forms attributes */
enum
{
    FL_FULLBORDER = 1,		/* normal */
    FL_TRANSIENT,		/* set TRANSIENT_FOR property             */
    FL_NOBORDER,		/* use override_redirect to supress decor. */
    FL_MODAL = 1 << 8		/* not implemented yet                    */
};

/* All box types */
typedef enum
{
    FL_NO_BOX,
    FL_UP_BOX,
    FL_DOWN_BOX,
    FL_BORDER_BOX,
    FL_SHADOW_BOX,
    FL_FRAME_BOX,
    FL_ROUNDED_BOX,
    FL_EMBOSSED_BOX,
    FL_FLAT_BOX,
    FL_RFLAT_BOX,
    FL_RSHADOW_BOX,
    FL_OVAL_BOX,
    FL_ROUNDED3D_UPBOX,
    FL_ROUNDED3D_DOWNBOX,
    FL_OVAL3D_UPBOX,
    FL_OVAL3D_DOWNBOX,
    FL_OVAL3D_FRAMEBOX,
    FL_OVAL3D_EMBOSSEDBOX,
    /* for internal use only */
    FL_TOPTAB_UPBOX,
    FL_SELECTED_TOPTAB_UPBOX,
    FL_BOTTOMTAB_UPBOX,
    FL_SELECTED_BOTTOMTAB_UPBOX,
    FL_OSHADOW_BOX		/* not used */
}
FL_BOX_TYPE;

#define FL_IS_UPBOX(t)    ((t) == FL_UP_BOX ||         \
                           (t) == FL_OVAL3D_UPBOX ||   \
                           (t) == FL_ROUNDED3D_UPBOX)

#define FL_IS_DOWNBOX(t)  ((t) == FL_DOWN_BOX ||       \
                           (t) == FL_OVAL3D_DOWNBOX || \
                           (t) == FL_ROUNDED3D_DOWNBOX)

#define FL_TO_DOWNBOX(t)  (t==FL_UP_BOX?FL_DOWN_BOX: \
                            (t==FL_ROUNDED3D_UPBOX ? FL_ROUNDED3D_DOWNBOX:\
                              (t==FL_OVAL3D_UPBOX?FL_OVAL3D_DOWNBOX:t)))

/* How to place text relative to a box */
typedef enum
{
    FL_ALIGN_CENTER,
    FL_ALIGN_TOP = 1,
    FL_ALIGN_BOTTOM = 2,
    FL_ALIGN_LEFT = 4,
    FL_ALIGN_RIGHT = 8,
    FL_ALIGN_TOP_LEFT = (FL_ALIGN_TOP | FL_ALIGN_LEFT),
    FL_ALIGN_TOP_RIGHT = (FL_ALIGN_TOP | FL_ALIGN_RIGHT),
    FL_ALIGN_BOTTOM_LEFT = (FL_ALIGN_BOTTOM | FL_ALIGN_LEFT),
    FL_ALIGN_BOTTOM_RIGHT = (FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT),
    FL_ALIGN_INSIDE = (1 << 13),
    FL_ALIGN_VERT = (1 << 14),	/* not functional yet  */

    FL_ALIGN_LEFT_TOP = FL_ALIGN_TOP_LEFT,
    FL_ALIGN_RIGHT_TOP = FL_ALIGN_TOP_RIGHT,
    FL_ALIGN_LEFT_BOTTOM = FL_ALIGN_BOTTOM_LEFT,
    FL_ALIGN_RIGHT_BOTTOM = FL_ALIGN_BOTTOM_RIGHT
}
FL_ALIGN;

/* mouse buttons. Don't have to be consecutive */
enum
{
    FL_MBUTTON1 = 1, FL_MBUTTON2, FL_MBUTTON3,
    FL_MBUTTON4, FL_MBUTTON5
};

#define FL_LEFT_MOUSE    FL_MBUTTON1
#define FL_MIDDLE_MOUSE  FL_MBUTTON2
#define FL_RIGHT_MOUSE   FL_MBUTTON3

#define FL_LEFTMOUSE     FL_LEFT_MOUSE
#define FL_MIDDLEMOUSE   FL_MIDDLE_MOUSE
#define FL_RIGHTMOUSE    FL_RIGHT_MOUSE

/* control when to reutrn input, slider and dial object. */
enum
{
    FL_RETURN_END_CHANGED = 0,
    FL_RETURN_CHANGED = 1,
    FL_RETURN_END = 2,
    FL_RETURN_ALWAYS = 3,
    FL_RETURN_DBLCLICK
};

/*
 *  Some special color indeces for FL private colormap. It does not matter
 *  what the value of each enum is, but it must start from 0 and be
 *  consecutive.
 */
typedef enum
{
    FL_BLACK, FL_RED, FL_GREEN, FL_YELLOW,
    FL_BLUE, FL_MAGENTA, FL_CYAN, FL_WHITE,

    FL_TOMATO, FL_INDIANRED, FL_SLATEBLUE,

    FL_COL1, FL_RIGHT_BCOL, FL_BOTTOM_BCOL, FL_TOP_BCOL, FL_LEFT_BCOL,
    FL_MCOL,

    FL_INACTIVE, FL_PALEGREEN, FL_DARKGOLD,

    FL_ORCHID, FL_DARKCYAN, FL_DARKTOMATO, FL_WHEAT, FL_DARKORANGE,
    FL_DEEPPINK, FL_CHARTREUSE, FL_DARKVIOLET, FL_SPRINGGREEN, FL_DODGERBLUE,

    FL_FREE_COL1 = 256, FL_FREE_COL2,
    FL_FREE_COL3, FL_FREE_COL4,
    FL_FREE_COL5, FL_FREE_COL6,
    FL_FREE_COL7, FL_FREE_COL8,
    FL_FREE_COL9, FL_FREE_COL10,
    FL_FREE_COL11, FL_FREE_COL12,
    FL_FREE_COL13, FL_FREE_COL14,
    FL_FREE_COL15, FL_FREE_COL16
}
FL_PD_COL;

#define FL_BUILT_IN_COLS  (FL_DOGERBLUE+1)
#define FL_INACTIVE_COL   FL_INACTIVE
#define FL_DOGERBLUE      FL_DODGERBLUE

/* Some aliases for the color. This is actually backwards ... */

#define FL_GRAY16           FL_RIGHT_BCOL
#define FL_GRAY35           FL_BOTTOM_BCOL
#define FL_GRAY80           FL_TOP_BCOL
#define FL_GRAY90           FL_LEFT_BCOL
#define FL_GRAY63           FL_COL1
#define FL_GRAY75           FL_MCOL

#define  FL_LCOL            FL_BLACK


/* Events that a form reacts to.  */
typedef enum
{
    FL_NOEVENT,
    FL_DRAW,
    FL_PUSH,
    FL_RELEASE,
    FL_ENTER,
    FL_LEAVE,
    FL_MOUSE,
    FL_FOCUS,
    FL_UNFOCUS,
    FL_KEYBOARD,
    FL_MOTION,
    FL_STEP,
    FL_SHORTCUT,
    FL_FREEMEM,
    FL_OTHER,			/* property, selection etc */
    FL_DRAWLABEL,
    FL_DBLCLICK,		/* double click            */
    FL_TRPLCLICK,		/* triple click            */
    FL_ATTRIB,			/* attribute change        */
    FL_PS			/* dump a form into EPS    */
}
FL_EVENTS;

#define FL_MOVE   FL_MOTION	/* for compatibility */

/* Resize policies */
typedef enum
{
    FL_RESIZE_NONE, FL_RESIZE_X, FL_RESIZE_Y
}
FL_RESIZE_T;

#define FL_RESIZE_ALL  (FL_RESIZE_X | FL_RESIZE_Y)

/* Keyboard focus control */
typedef enum
{
    FL_KEY_NORMAL = 1,		/* normal keys(0-255) - tab +left/right */
    FL_KEY_TAB = 2,		/* normal keys + 4 direction cursor     */
    FL_KEY_SPECIAL = 4,		/* only needs special keys(>255)        */
    FL_KEY_ALL = 7		/* all keys                             */
}
FL_KEY;

#define FL_ALT_MASK      (1L<<25)	/* alt + Key --> FL_ALT_VAL + key */
#define FL_CONTROL_MASK  (1L<<26)
#define FL_SHIFT_MASK    (1L<<27)
#define FL_ALT_VAL       FL_ALT_MASK

/* Internal use */

typedef enum
{
    FL_FIND_INPUT,
    FL_FIND_AUTOMATIC,
    FL_FIND_MOUSE,
    FL_FIND_CANVAS,
    FL_FIND_KEYSPECIAL
}
FL_FIND;

/*
 *  Pop-up menu item attributes. NOTE if more than 8, need to change
 *  choice and menu class where mode is kept by a single byte
 */
enum
{
    FL_PUP_NONE,
    FL_PUP_GREY = 1,
    FL_PUP_BOX = 2,
    FL_PUP_CHECK = 4,
    FL_PUP_RADIO = 8
};

#define FL_PUP_GRAY      FL_PUP_GREY
#define FL_PUP_TOGGLE    FL_PUP_BOX
#define FL_PUP_INACTIVE  FL_PUP_GREY

/* popup and menu entries */

typedef int (*FL_PUP_CB) (int);	/* call back prototype  */

typedef struct
{
    const char *text;		/* label of a popup/menu item    */
    FL_PUP_CB callback;		/* the callback function         */
    const char *shortcut;	/* hotkeys                       */
    int mode;			/* FL_PUP_GRAY, FL_PUP_CHECK etc */
}
FL_PUP_ENTRY;

#define FL_MENU_ENTRY  FL_PUP_ENTRY


/*******************************************************************
 * FONTS
 ******************************************************************/

#define FL_MAXFONTS     48	/* max number of fonts */

typedef enum
{
    FL_INVALID_STYLE = -1,
    FL_NORMAL_STYLE,
    FL_BOLD_STYLE,
    FL_ITALIC_STYLE,
    FL_BOLDITALIC_STYLE,

    FL_FIXED_STYLE,
    FL_FIXEDBOLD_STYLE,
    FL_FIXEDITALIC_STYLE,
    FL_FIXEDBOLDITALIC_STYLE,

    FL_TIMES_STYLE,
    FL_TIMESBOLD_STYLE,
    FL_TIMESITALIC_STYLE,
    FL_TIMESBOLDITALIC_STYLE,

    /* modfier masks. Need to fit a short  */
    FL_SHADOW_STYLE = (1 << 9),
    FL_ENGRAVED_STYLE = (1 << 10),
    FL_EMBOSSED_STYLE = (1 << 11)
}
FL_TEXT_STYLE;


#define FL_FONT_STYLE FL_TEXT_STYLE

#define special_style(a)  (a >=FL_SHADOW_STYLE &&\
                           a <= (FL_EMBOSSED_STYLE+FL_MAXFONTS))

/* Standard sizes in XForms */
#define FL_TINY_SIZE       8
#define FL_SMALL_SIZE      10
#define FL_NORMAL_SIZE     12
#define FL_MEDIUM_SIZE     14
#define FL_LARGE_SIZE      18
#define FL_HUGE_SIZE       24

#define FL_DEFAULT_SIZE   FL_SMALL_SIZE

/* Defines for compatibility */
#define FL_TINY_FONT      FL_TINY_SIZE
#define FL_SMALL_FONT     FL_SMALL_SIZE
#define FL_NORMAL_FONT    FL_NORMAL_SIZE
#define FL_MEDIUM_FONT    FL_MEDIUM_SIZE
#define FL_LARGE_FONT     FL_LARGE_SIZE
#define FL_HUGE_FONT      FL_HUGE_SIZE

#define FL_NORMAL_FONT1   FL_SMALL_FONT
#define FL_NORMAL_FONT2   FL_NORMAL_FONT
#define FL_DEFAULT_FONT   FL_SMALL_FONT

#define FL_BOUND_WIDTH  (FL_Coord)3	/* Border width of boxes */

/*
 *  Definition of basic struct that holds an object
 */

#define  FL_CLICK_TIMEOUT  400	/* double click interval */

struct forms_;
struct fl_pixmap;

typedef struct flobjs_
{
    struct forms_ *form;	/* the form this object belongs to     */
    void *u_vdata;		/* anything the user likes             */
    char *u_cdata;		/* anything the user likes             */
    long u_ldata;		/* anything the user likes             */

    int objclass;		/* class of object, button, slider etc */
    int type;			/* type within the class               */
    int boxtype;		/* what kind of box type               */
    FL_Coord x, y, w, h;	/* obj. location and size              */
    FL_Coord bw;
    FL_COLOR col1, col2;	/* colors of obj                       */
    char *label;		/* object label                        */
    FL_COLOR lcol;		/* label color                         */
    int align;
    int lsize, lstyle;		/* label size and style                */
    long *shortcut;
    int (*handle) (struct flobjs_ *, int, FL_Coord, FL_Coord, int, void *);
    void (*object_callback) (struct flobjs_ *, long);
    long argument;
    void *spec;			/* instantiation                      */

    int (*prehandle) (struct flobjs_ *, int, FL_Coord, FL_Coord, int, void *);
    int (*posthandle) (struct flobjs_ *, int, FL_Coord, FL_Coord, int, void *);

    /* re-configure preference */
    unsigned int resize;	/* what to do if WM resizes the FORM     */
    unsigned int nwgravity;	/* how to re-position top-left corner    */
    unsigned int segravity;	/* how to re-position lower-right corner */

    struct flobjs_ *prev;	/* prev. obj in form                     */
    struct flobjs_ *next;	/* next. obj in form                     */

    struct flobjs_ *parent;
    struct flobjs_ *child;
    struct flobjs_ *nc;
    int is_child;

    void *flpixmap;		/* pixmap double buffering stateinfo     */
    int use_pixmap;		/* true to use pixmap double buffering   */

    /* some interaction flags */
    int double_buffer;		/* only used by mesa/gl canvas           */
    int pushed;
    int focus;
    int belowmouse;
    int active;			/* if accept event */
    int input;
    int wantkey;
    int radio;
    int automatic;
    int redraw;
    int visible;
    int clip;
    unsigned long click_timeout;
    void *c_vdata;		/* for class use            */
    char *c_cdata;		/* for class use            */
    long c_ldata;		/* for class use            */
    unsigned int spec_size;	/* for internal use         */
    FL_COLOR aux_col1, aux_col2;	/* aux colors               */
    FL_COLOR dbl_background;	/* double buffer background */
    int how_return;
    int reserved[6];		/* for future use           */
}
FL_OBJECT;

/* callback function for an entire form */
typedef void (*FL_FORMCALLBACKPTR) (struct flobjs_ *, void *);

/* object callback function      */
typedef void (*FL_CALLBACKPTR) (FL_OBJECT *, long);

/* preemptive callback function  */
typedef int (*FL_RAW_CALLBACK) (struct forms_ *, void *);

/* at close (WM menu delete/close etc.) */
typedef int (*FL_FORM_ATCLOSE) (struct forms_ *, void *);

/* deactivate/activate callback */
typedef void (*FL_FORM_ATDEACTIVATE) (struct forms_ *, void *);
typedef void (*FL_FORM_ATACTIVATE) (struct forms_ *, void *);

typedef int (*FL_HANDLEPTR) (FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);

FL_EXPORT FL_OBJECT *FL_EVENT;

/*** FORM ****/

typedef struct forms_
{
    void *fdui;			/* for fdesign              */
    void *u_vdata;		/* for application          */
    char *u_cdata;		/* for application          */
    long u_ldata;		/* for application          */

    char *label;		/* window title             */
    unsigned long window;	/* X resource ID for window */
    FL_Coord x, y, w, h;	/* current geometry info    */
    FL_Coord hotx, hoty;	/* hot-spot of the form     */

    struct flobjs_ *first;
    struct flobjs_ *last;
    struct flobjs_ *focusobj;

    FL_FORMCALLBACKPTR form_callback;
    FL_FORM_ATACTIVATE activate_callback;
    FL_FORM_ATDEACTIVATE deactivate_callback;
    void *form_cb_data, *activate_data, *deactivate_data;

    FL_RAW_CALLBACK key_callback;
    FL_RAW_CALLBACK push_callback;
    FL_RAW_CALLBACK crossing_callback;
    FL_RAW_CALLBACK motion_callback;
    FL_RAW_CALLBACK all_callback;

    unsigned long compress_mask;
    unsigned long evmask;

    /* WM_DELETE_WINDOW message handler */
    FL_FORM_ATCLOSE close_callback;
    void *close_data;

    void *flpixmap;		/* back buffer             */

    unsigned long icon_pixmap;
    unsigned long icon_mask;

    /* interaction and other flags */
    int vmode;			/* current X visual class  */
    int deactivated;		/* true if sensitive       */
    int use_pixmap;		/* true if dbl buffering   */
    int frozen;			/* true if sync change     */
    int visible;		/* true if mapped          */
    int wm_border;		/* window manager info     */
    unsigned int prop;		/* other attributes        */
    int has_auto;
    int top;
    int sort_of_modal;		/* internal use.           */
    struct forms_ *parent;
    struct forms_ *child;
    struct flobjs_ *parent_obj;
    int reserved[10];		/* future use              */
}
FL_FORM;

/*
 * All FD_xxx structure emitted by fdesign contains at least the
 * following
 */
typedef struct
{
    FL_FORM *form;
    void *vdata;
    char *cdata;
    long ldata;
}
FD_Any;


/*
 * Async IO stuff
 */

enum
{
    FL_READ = 1, FL_WRITE = 2, FL_EXCEPT = 4
};

/* IO other than XEvent Q */
typedef void (*FL_IO_CALLBACK) (int, void *);
FL_EXPORT void fl_add_io_callback(int, unsigned, FL_IO_CALLBACK, void *);
FL_EXPORT void fl_remove_io_callback(int, unsigned, FL_IO_CALLBACK);

/* signals */

typedef void (*FL_SIGNAL_HANDLER) (int, void *);
FL_EXPORT void fl_add_signal_callback(int, FL_SIGNAL_HANDLER, void *);
FL_EXPORT void fl_remove_signal_callback(int);
FL_EXPORT void fl_signal_caught(int);
FL_EXPORT void fl_app_signal_direct(int);

/* timeouts */
FL_EXPORT int fl_add_timeout(long, void (*)(int, void *), void *);
FL_EXPORT void fl_remove_timeout(int);

/*  Some utility stuff */
typedef struct
{
    int val;
    const char *name;
}
FL_VN_PAIR;

FL_EXPORT int fl_get_vn_value(FL_VN_PAIR *, const char *);
FL_EXPORT const char *fl_get_vn_name(FL_VN_PAIR *, int);
FL_EXPORT unsigned long fl_msleep(unsigned long);

/*
 *  Basic public routine prototypes
 */

FL_EXPORT int fl_library_version(int *, int *);

/** Generic routines that deal with FORMS **/

FL_EXPORT FL_FORM *fl_bgn_form(int, FL_Coord, FL_Coord);
FL_EXPORT void fl_end_form(void);
FL_EXPORT FL_OBJECT *fl_do_forms(void);
FL_EXPORT FL_OBJECT *fl_check_forms(void);
FL_EXPORT FL_OBJECT *fl_do_only_forms(void);
FL_EXPORT FL_OBJECT *fl_check_only_forms(void);
FL_EXPORT void fl_freeze_form(FL_FORM *);

FL_EXPORT void fl_set_focus_object(FL_FORM *, FL_OBJECT *);
FL_EXPORT FL_OBJECT *fl_get_focus_object(FL_FORM *);
FL_EXPORT void fl_reset_focus_object(FL_OBJECT *);
#define fl_set_object_focus   fl_set_focus_object

FL_EXPORT FL_FORM_ATCLOSE fl_set_form_atclose(FL_FORM *, FL_FORM_ATCLOSE, void *);
FL_EXPORT FL_FORM_ATCLOSE fl_set_atclose(FL_FORM_ATCLOSE, void *);

FL_EXPORT FL_FORM_ATACTIVATE fl_set_form_atactivate(FL_FORM *,
						FL_FORM_ATACTIVATE, void *);
FL_EXPORT FL_FORM_ATDEACTIVATE fl_set_form_atdeactivate(FL_FORM *,
					      FL_FORM_ATDEACTIVATE, void *);

FL_EXPORT void fl_unfreeze_form(FL_FORM *);
FL_EXPORT void fl_deactivate_form(FL_FORM *);
FL_EXPORT void fl_activate_form(FL_FORM *);
FL_EXPORT void fl_deactivate_all_forms(void);
FL_EXPORT void fl_activate_all_forms(void);
FL_EXPORT void fl_freeze_all_forms(void);
FL_EXPORT void fl_unfreeze_all_forms(void);
FL_EXPORT void fl_scale_form(FL_FORM *, double, double);
FL_EXPORT void fl_set_form_position(FL_FORM *, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_form_title(FL_FORM *, const char *);

FL_EXPORT void fl_set_form_property(FL_FORM *, unsigned);
FL_EXPORT void fl_set_app_mainform(FL_FORM *);
FL_EXPORT FL_FORM *fl_get_app_mainform(void);
FL_EXPORT void fl_set_app_nomainform(int);

FL_EXPORT void fl_set_form_callback(FL_FORM *, FL_FORMCALLBACKPTR, void *);
#define  fl_set_form_call_back    fl_set_form_callback

FL_EXPORT void fl_set_form_size(FL_FORM *, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_form_hotspot(FL_FORM *, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_form_hotobject(FL_FORM *, FL_OBJECT *);
FL_EXPORT void fl_set_form_minsize(FL_FORM *, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_form_maxsize(FL_FORM *, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_form_event_cmask(FL_FORM *, unsigned long);
FL_EXPORT unsigned long fl_get_form_event_cmask(FL_FORM *);

FL_EXPORT void fl_set_form_geometry(FL_FORM *, FL_Coord, FL_Coord,
				    FL_Coord, FL_Coord);

#define fl_set_initial_placement fl_set_form_geometry

FL_EXPORT long fl_show_form(FL_FORM *, int, int, const char *);
FL_EXPORT void fl_hide_form(FL_FORM *);
FL_EXPORT void fl_free_form(FL_FORM *);
FL_EXPORT void fl_redraw_form(FL_FORM *);
FL_EXPORT void fl_set_form_dblbuffer(FL_FORM *, int);
FL_EXPORT long fl_prepare_form_window(FL_FORM *, int, int, const char *);
FL_EXPORT long fl_show_form_window(FL_FORM *);
FL_EXPORT double fl_adjust_form_size(FL_FORM *);
FL_EXPORT int fl_form_is_visible(FL_FORM *);

FL_EXPORT FL_RAW_CALLBACK fl_register_raw_callback(FL_FORM *, unsigned long,
						   FL_RAW_CALLBACK);

#define fl_register_call_back fl_register_raw_callback

FL_EXPORT FL_OBJECT *fl_bgn_group(void);
FL_EXPORT FL_OBJECT *fl_end_group(void);
FL_EXPORT void fl_addto_group(FL_OBJECT *);

/****** Routines that deal with FL_OBJECTS ********/

FL_EXPORT void fl_set_object_boxtype(FL_OBJECT *, int);
FL_EXPORT void fl_set_object_bw(FL_OBJECT *, int);
FL_EXPORT void fl_set_object_resize(FL_OBJECT *, unsigned);
FL_EXPORT void fl_set_object_gravity(FL_OBJECT *, unsigned, unsigned);
FL_EXPORT void fl_set_object_lsize(FL_OBJECT *, int);
FL_EXPORT void fl_set_object_lstyle(FL_OBJECT *, int);
FL_EXPORT void fl_set_object_lcol(FL_OBJECT *, FL_COLOR);
FL_EXPORT void fl_set_object_return(FL_OBJECT *, int);
FL_EXPORT void fl_set_object_lalign(FL_OBJECT *, int);	/* to be removed */
FL_EXPORT void fl_set_object_shortcut(FL_OBJECT *, const char *, int);
FL_EXPORT void fl_set_object_shortcutkey(FL_OBJECT *, unsigned int);
FL_EXPORT void fl_set_object_dblbuffer(FL_OBJECT *, int);
FL_EXPORT void fl_set_object_color(FL_OBJECT *, FL_COLOR, FL_COLOR);
FL_EXPORT void fl_set_object_label(FL_OBJECT *, const char *);
FL_EXPORT void fl_set_object_position(FL_OBJECT *, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_object_size(FL_OBJECT *, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_object_automatic(FL_OBJECT *, int);
FL_EXPORT void fl_draw_object_label(FL_OBJECT *);
FL_EXPORT void fl_draw_object_label_outside(FL_OBJECT *);
FL_EXPORT FL_OBJECT *fl_get_object_component(FL_OBJECT *, int, int, int);
#define fl_draw_object_outside_label fl_draw_object_label_outside
#define  fl_set_object_dblclick(ob, timeout)  (ob)->click_timeout = (timeout);
FL_EXPORT void fl_set_object_geometry(FL_OBJECT *, FL_Coord, FL_Coord,
				      FL_Coord, FL_Coord);

#define fl_set_object_lcolor  fl_set_object_lcol

FL_EXPORT void fl_fit_object_label(FL_OBJECT *, FL_Coord, FL_Coord);

FL_EXPORT void fl_get_object_geometry(FL_OBJECT * ob, FL_Coord *, FL_Coord *,
				      FL_Coord *, FL_Coord *);

FL_EXPORT void fl_get_object_position(FL_OBJECT *, FL_Coord *, FL_Coord *);

/* this one takes into account the label */
FL_EXPORT void fl_get_object_bbox(FL_OBJECT *, FL_Coord *, FL_Coord *,
				  FL_Coord *, FL_Coord *);

#define fl_compute_object_geometry   fl_get_object_bbox

FL_EXPORT void fl_call_object_callback(FL_OBJECT *);
FL_EXPORT FL_HANDLEPTR fl_set_object_prehandler(FL_OBJECT *, FL_HANDLEPTR);
FL_EXPORT FL_HANDLEPTR fl_set_object_posthandler(FL_OBJECT *, FL_HANDLEPTR);
FL_EXPORT FL_CALLBACKPTR fl_set_object_callback(FL_OBJECT *, FL_CALLBACKPTR, long);

#define fl_set_object_align   fl_set_object_lalign
#define fl_set_call_back      fl_set_object_callback

FL_EXPORT void fl_redraw_object(FL_OBJECT *);
FL_EXPORT void fl_scale_object(FL_OBJECT *, double, double);
FL_EXPORT void fl_show_object(FL_OBJECT *);
FL_EXPORT void fl_hide_object(FL_OBJECT *);
FL_EXPORT void fl_free_object(FL_OBJECT *);
FL_EXPORT void fl_delete_object(FL_OBJECT *);
FL_EXPORT void fl_trigger_object(FL_OBJECT *);
FL_EXPORT void fl_activate_object(FL_OBJECT *);
FL_EXPORT void fl_deactivate_object(FL_OBJECT *);

FL_EXPORT int fl_enumerate_fonts(void (*)(const char *s), int);
FL_EXPORT int fl_set_font_name(int, const char *);
FL_EXPORT void fl_set_font(int, int);

/* routines that facilitate free object */

FL_EXPORT int fl_get_char_height(int, int, int *, int *);
FL_EXPORT int fl_get_char_width(int, int);
FL_EXPORT int fl_get_string_height(int, int, const char *, int, int *, int *);
FL_EXPORT int fl_get_string_width(int, int, const char *, int);
FL_EXPORT int fl_get_string_widthTAB(int, int, const char *, int);
FL_EXPORT void fl_get_string_dimension(int, int, const char *, int, int *, int *);

#define fl_get_string_size  fl_get_string_dimension

FL_EXPORT void fl_get_align_xy(int, int, int, int, int, int, int,
			       int, int, int *, int *);

FL_EXPORT void fl_drw_text(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			   FL_COLOR, int, int, char *);

FL_EXPORT void fl_drw_text_beside(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				  FL_COLOR, int, int, char *);

#define fl_draw_text(a,x,y,w,h,c,st,sz,s)    \
      (((a) & FL_ALIGN_INSIDE) ? fl_drw_text:fl_drw_text_beside)\
      (a,x,y,w,h,c,st,sz,s)

FL_EXPORT void fl_drw_text_cursor(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				  FL_COLOR, int, int, char *, int, int);

FL_EXPORT void fl_drw_box(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			  FL_COLOR, int);

typedef void (*FL_DRAWPTR) (FL_Coord x, FL_Coord y, FL_Coord w, FL_Coord h,
			    int, FL_COLOR);
FL_EXPORT int fl_add_symbol(const char *, FL_DRAWPTR, int);
FL_EXPORT int fl_draw_symbol(const char *, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			     FL_COLOR);
enum
{
    FL_SLIDER_NONE = 0,
    FL_SLIDER_BOX = 1,
    FL_SLIDER_KNOB = 2,
    FL_SLIDER_UP = 4,
    FL_SLIDER_DOWN = 8,
    FL_SLIDER_ALL = 15
};
FL_EXPORT void fl_drw_slider(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			     FL_COLOR, FL_COLOR, int, double, double, char *,
			     int, int, int);

FL_EXPORT unsigned long fl_mapcolor(FL_COLOR, int, int, int);
FL_EXPORT long fl_mapcolorname(FL_COLOR, const char *);
#define fl_mapcolor_name  fl_mapcolorname

FL_EXPORT void fl_free_colors(FL_COLOR *, int);
FL_EXPORT void fl_free_pixels(unsigned long *, int);
FL_EXPORT void fl_set_color_leak(int);
FL_EXPORT unsigned long fl_getmcolor(FL_COLOR, int *, int *, int *);
FL_EXPORT unsigned long fl_get_pixel(FL_COLOR);
#define fl_get_flcolor   fl_get_pixel

FL_EXPORT void fl_get_icm_color(FL_COLOR, int *, int *, int *);
FL_EXPORT void fl_set_icm_color(FL_COLOR, int, int, int);

FL_EXPORT void fl_color(FL_COLOR);
FL_EXPORT void fl_bk_color(FL_COLOR);
FL_EXPORT void fl_textcolor(FL_COLOR);
FL_EXPORT void fl_bk_textcolor(FL_COLOR);
FL_EXPORT void fl_set_gamma(double, double, double);

FL_EXPORT void fl_show_errors(int);
FL_EXPORT void fl_set_error_handler(void (*)(const char *, const char *,...));
FL_EXPORT void fl_set_error_logfp(FILE * fp);

/* Some macros */
#define FL_max(a,b)      ( (a) > (b) ? (a):(b) )
#define FL_min(a,b)      ( (a) < (b) ? (a):(b) )
#define FL_abs(a)        ( (a) > 0 ? (a):(-(a)))
#define FL_nint(a)       ( (a) > 0 ? ((a) + 0.5):((a) - 0.5))

typedef int (*FL_FSCB) (const char *, void *);

/* utilities for new objects */
FL_EXPORT FL_FORM *fl_current_form;
FL_EXPORT void fl_add_object(FL_FORM *, FL_OBJECT *);
FL_EXPORT void fl_addto_form(FL_FORM *);
FL_EXPORT FL_OBJECT *fl_make_object(int, int, FL_Coord, FL_Coord,
			    FL_Coord, FL_Coord, const char *, FL_HANDLEPTR);

FL_EXPORT void fl_set_coordunit(int);
FL_EXPORT void fl_set_border_width(int);
FL_EXPORT void fl_set_scrollbar_type(int);
#define fl_set_thinscrollbar(t)   fl_set_scrollbar_type(\
                                  t ? FL_THIN_SCROLLBAR:FL_NORMAL_SCROLLBAR)

FL_EXPORT void fl_flip_yorigin(void);

FL_EXPORT int fl_get_coordunit(void);
FL_EXPORT int fl_get_border_width(void);

/* misc. routines */
FL_EXPORT void fl_ringbell(int percent);
FL_EXPORT void fl_gettime(long *sec, long *usec);
FL_EXPORT const char *fl_now(void);
FL_EXPORT const char *fl_whoami(void);
FL_EXPORT long fl_mouse_button(void);
#define fl_mousebutton fl_mouse_button

/* this gives more flexibility for future changes */

#define fl_free       free
#define fl_malloc     malloc
#define fl_calloc     calloc
#define fl_realloc    realloc

#define FL_MAX_MENU_CHOICE_ITEMS   128

#endif /* Basic.h *} */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *  X Window dependent stuff
 *
 */

#ifndef FL_XBASIC_H
#define FL_XBASIC_H		/* { */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#ifdef FL_WIN32
#include <X11/Xwinsock.h>
#endif

/* draw mode */
enum
{
    FL_XOR = GXxor,
    FL_COPY = GXcopy,
    FL_AND = GXand
};

#define FL_MINDEPTH  1

/* FL_xxx does not do anything anymore, but kept for compatibility */
enum
{
    FL_illegalVisual = -1,
    FL_StaticGray = StaticGray,
    FL_GrayScale = GrayScale,
    FL_StaticColor = StaticColor,
    FL_PseudoColor = PseudoColor,
    FL_TrueColor = TrueColor,
    FL_DirectColor = DirectColor,
    FL_DefaultVisual = 10	/* special request */
};

enum
{
    FL_North = NorthGravity,
    FL_NorthEast = NorthEastGravity,
    FL_NorthWest = NorthWestGravity,
    FL_South = SouthGravity,
    FL_SouthEast = SouthEastGravity,
    FL_SouthWest = SouthWestGravity,
    FL_East = EastGravity,
    FL_West = WestGravity,
    FL_NoGravity = ForgetGravity,
    FL_ForgetGravity = ForgetGravity
};

#ifndef GreyScale
#define GreyScale   GrayScale
#define StaticGrey  StaticGray
#endif

#define FL_is_gray(v)  (v==GrayScale || v==StaticGray)
#define FL_is_rgb(v)   (v==TrueColor || v==DirectColor)


/*
 * Internal colormap size. Not really very meaningful as fl_mapcolor
 * and company allow color "leakage", that is, although only FL_MAX_COLS
 * are kept in the internal colormap, the server might have substantially
 * more colors allocated
 */

#define FL_MAX_COLS   1024

/*
 * FL graphics state information. Some are redundant.
 */

typedef struct
{
    XVisualInfo *xvinfo;
    XFontStruct *cur_fnt;	/* current font in default GC       */
    Colormap colormap;		/* colormap valid for xvinfo        */
    Window trailblazer;		/* a valid window for xvinfo        */
    int vclass, depth;		/* visual class and color depth     */
    int rgb_bits;		/* primary color resolution         */
    int dithered;		/* true if dithered color           */
    int pcm;			/* true if colormap is not shared   */
    GC gc[16];			/* working GC                       */
    GC textgc[16];		/* GC used exclusively for text     */
    GC dimmedGC;		/* A GC having a checkboard stipple */
    unsigned long lut[FL_MAX_COLS];	/* secondary lookup table         */
    unsigned int rshift, rmask, rbits;
    unsigned int gshift, gmask, gbits;
    unsigned int bshift, bmask, bbits;
}
FL_STATE;

#define FL_State FL_STATE	/* for compatibility */

/***** Global variables ******/

FL_EXPORT Display *fl_display;
FL_EXPORT int fl_screen;
FL_EXPORT Window fl_root;	/* root window                */
FL_EXPORT Window fl_vroot;	/* virtual root window        */
FL_EXPORT int fl_scrh, fl_scrw;	/* screen dimension in pixels */
FL_EXPORT int fl_vmode;

/* current version only runs in single visual mode */
#define  fl_get_vclass()        fl_vmode
#define  fl_get_form_vclass(a)  fl_vmode
#define  fl_get_gc()            fl_state[fl_vmode].gc[0]

FL_EXPORT FL_State fl_state[];
FL_EXPORT char *fl_ul_magic_char;
FL_EXPORT int fl_mode_capable(int /* mode */ , int /* warn */ );

#define fl_default_win()       (fl_state[fl_vmode].trailblazer)
#define fl_default_window()    (fl_state[fl_vmode].trailblazer)
/*
 * All pixmaps used by FL_OBJECT to simulate double buffering have the
 * following entries in the structure. FL_Coord x,y are used to shift
 * the origin of the drawing routines
 */
typedef struct
{
    Pixmap pixmap;
    Window win;
    Visual *visual;
    FL_Coord x, y;
    unsigned int w, h;
    int depth;
    FL_COLOR dbl_background;
    FL_COLOR pixel;
}
FL_pixmap;


/* fonts related */

#define FL_MAX_FONTSIZES   10

typedef struct
{
    XFontStruct *fs[FL_MAX_FONTSIZES];	/* cached fontstruct */
    short size[FL_MAX_FONTSIZES];	/* cached sizes      */
    short nsize;		/* cached so far     */
    char fname[80];		/* without size info     */
}
FL_FONT;

/*
 * Some basic drawing routines
 */

typedef XPoint FL_POINT;
typedef XRectangle FL_RECT;

/* rectangles */
FL_EXPORT void fl_rectangle(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, FL_COLOR);
FL_EXPORT void fl_rectbound(FL_Coord, FL_Coord, FL_Coord, FL_Coord, FL_COLOR);
#define fl_rectf(x,y,w,h,c)   fl_rectangle(1, x,y,w,h,c)
#define fl_rect(x,y,w,h,c)    fl_rectangle(0, x,y,w,h,c)

/* rectangle with rounded-corners */
FL_EXPORT void fl_roundrectangle(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, FL_COLOR);
#define fl_roundrectf(x,y,w,h,c) fl_roundrectangle(1,x,y,w,h,c)
#define fl_roundrect(x,y,w,h,c) fl_roundrectangle(0,x,y,w,h,c)

/* general polygon and polylines */
FL_EXPORT void fl_polygon(int, FL_POINT *, int n, FL_COLOR);
#define fl_polyf(p,n,c)  fl_polygon(1, p, n, c)
#define fl_polyl(p,n,c)  fl_polygon(0, p, n, c)
#define fl_polybound(p,n,c) do {fl_polyf(p,n,c);fl_polyl(p,n,FL_BLACK);}while(0)

FL_EXPORT void fl_lines(FL_POINT *, int n, FL_COLOR);
FL_EXPORT void fl_line(FL_Coord, FL_Coord, FL_Coord, FL_Coord, FL_COLOR);
FL_EXPORT void fl_point(FL_Coord, FL_Coord, FL_COLOR);
FL_EXPORT void fl_points(FL_POINT *, int, FL_COLOR);
#define fl_simple_line fl_line

FL_EXPORT void fl_dashedlinestyle(const char *, int);

#define fl_diagline(x,y,w,h,c) fl_line(x,y,(x)+(w)-1,(y)+(h)-1,c)

/* line attributes */
enum
{
    FL_SOLID = LineSolid,
    FL_USERDASH = LineOnOffDash,
    FL_USERDOUBLEDASH = LineDoubleDash,
    FL_DOT,
    FL_DOTDASH,
    FL_DASH,
    FL_LONGDASH
};
FL_EXPORT void fl_linewidth(int);
FL_EXPORT void fl_linestyle(int);
FL_EXPORT void fl_drawmode(int);

FL_EXPORT int fl_get_linewidth(void);
FL_EXPORT int fl_get_linestyle(void);
FL_EXPORT int fl_get_drawmode(void);

#define fl_set_linewidth    fl_linewidth
#define fl_set_linestyle    fl_linestyle
#define fl_set_drawmode     fl_drawmode


/** ellipses **/
FL_EXPORT void fl_oval(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, FL_COLOR);
FL_EXPORT void fl_ovalbound(FL_Coord, FL_Coord, FL_Coord, FL_Coord, FL_COLOR);
FL_EXPORT void fl_ovalarc(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			  int, int, FL_COLOR);

#define fl_ovalf(x,y,w,h,c)     fl_oval(1,x,y,w,h,c)
#define fl_ovall(x,y,w,h,c)     fl_oval(0,x,y,w,h,c)
#define fl_oval_bound           fl_ovalbound

#define fl_circf(x,y,r,col)  fl_oval(1,(x)-(r),(y)-(r),2*(r),2*(r),col)
#define fl_circ(x,y,r,col)   fl_oval(0,(x)-(r),(y)-(r),2*(r),2*(r),col)

/* arcs */
FL_EXPORT void fl_pieslice(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			   int, int, FL_COLOR);

#define fl_arcf(x,y,r,a1,a2,c)  fl_pieslice(1,(x)-(r),(y)-(r),\
                                (2*(r)),(2*(r)), a1,a2,c)

#define fl_arc(x,y,r,a1,a2,c)  fl_pieslice(0,(x)-(r),(y)-(r), \
                               (2*(r)),(2*(r)), a1,a2,c)

/* misc. stuff */
FL_EXPORT void fl_add_vertex(FL_Coord, FL_Coord);
FL_EXPORT void fl_add_float_vertex(float, float);
FL_EXPORT void fl_reset_vertex(void);
FL_EXPORT void fl_endline(void), fl_endpolygon(void), fl_endclosedline(void);

#define fl_bgnline       fl_reset_vertex
#define fl_bgnclosedline fl_reset_vertex
#define fl_bgnpolygon    fl_reset_vertex
#define fl_v2s(v)        fl_add_vertex(v[0], v[1])
#define fl_v2i(v)        fl_add_vertex(v[0], v[1])
#define fl_v2f(v)        fl_add_float_vertex(v[0], v[1])
#define fl_v2d(v)        fl_add_float_vertex(v[0], v[1])

/* high level drawing routines */
FL_EXPORT void fl_drw_frame(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			    FL_COLOR, int);
FL_EXPORT void fl_drw_checkbox(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			       FL_COLOR, int);

/*
 * Interfaces
 */
FL_EXPORT XFontStruct *fl_get_fontstruct(int, int);
#define fl_get_font_struct fl_get_fontstruct
#define fl_get_fntstruct fl_get_font_struct

FL_EXPORT Window fl_get_mouse(FL_Coord *, FL_Coord *, unsigned int *);
FL_EXPORT void fl_set_mouse(FL_Coord, FL_Coord);
FL_EXPORT Window fl_get_win_mouse(Window, FL_Coord *, FL_Coord *, unsigned *);
FL_EXPORT Window fl_get_form_mouse(FL_FORM *, FL_Coord *, FL_Coord *, unsigned *);
FL_EXPORT FL_FORM *fl_win_to_form(Window);
FL_EXPORT void fl_set_form_icon(FL_FORM *, Pixmap, Pixmap);

#define fl_raise_form(f) do {if(f->window) XRaiseWindow(fl_display,f->window);}while(0)
#define fl_lower_form(f) do {if(f->window) XLowerWindow(fl_display,f->window);}while(0)

#define fl_set_foreground(gc,c) XSetForeground(fl_display,gc,fl_get_pixel(c))
#define fl_set_background(gc,c) XSetBackground(fl_display,gc,fl_get_pixel(c))

/* General windowing support */

FL_EXPORT Window fl_wincreate(const char *);
FL_EXPORT Window fl_winshow(Window);
FL_EXPORT Window fl_winopen(const char *);
FL_EXPORT void fl_winhide(Window);
FL_EXPORT void fl_winclose(Window);
FL_EXPORT void fl_winset(Window);
FL_EXPORT int fl_winreparent(Window, Window);
FL_EXPORT void fl_winfocus(Window win);
FL_EXPORT Window fl_winget(void);

FL_EXPORT void fl_winresize(Window, FL_Coord, FL_Coord);
FL_EXPORT void fl_winmove(Window, FL_Coord, FL_Coord);
FL_EXPORT void fl_winreshape(Window, FL_Coord, FL_Coord, FL_Coord, FL_Coord);
FL_EXPORT void fl_winicon(Window, Pixmap, Pixmap);
FL_EXPORT void fl_winbackground(Window, unsigned long);
FL_EXPORT void fl_winstepunit(Window, FL_Coord, FL_Coord);
FL_EXPORT int fl_winisvalid(Window);
FL_EXPORT void fl_wintitle(Window, const char *);
FL_EXPORT void fl_winicontitle(Window, const char *);
FL_EXPORT void fl_winposition(FL_Coord, FL_Coord);

#define fl_pref_winposition fl_winposition
#define fl_win_background     fl_winbackground
#define fl_set_winstepunit    fl_winstepunit


FL_EXPORT void fl_winminsize(Window, FL_Coord, FL_Coord);
FL_EXPORT void fl_winmaxsize(Window, FL_Coord, FL_Coord);
FL_EXPORT void fl_winaspect(Window, FL_Coord, FL_Coord);
FL_EXPORT void fl_reset_winconstraints(Window);

FL_EXPORT void fl_winsize(FL_Coord, FL_Coord);
FL_EXPORT void fl_initial_winsize(FL_Coord, FL_Coord);
#define fl_pref_winsize  fl_winsize

FL_EXPORT void fl_initial_winstate(int);

FL_EXPORT Colormap fl_create_colormap(XVisualInfo *, int);


FL_EXPORT void fl_wingeometry(FL_Coord, FL_Coord, FL_Coord, FL_Coord);
#define fl_pref_wingeometry  fl_wingeometry
FL_EXPORT void fl_initial_wingeometry(FL_Coord, FL_Coord, FL_Coord, FL_Coord);

FL_EXPORT void fl_noborder(void);
FL_EXPORT void fl_transient(void);

FL_EXPORT void fl_get_winsize(Window, FL_Coord *, FL_Coord *);
FL_EXPORT void fl_get_winorigin(Window, FL_Coord *, FL_Coord *);
FL_EXPORT void fl_get_wingeometry(Window, FL_Coord *, FL_Coord *,
				  FL_Coord *, FL_Coord *);

/* for compatibility */
#define fl_get_win_size          fl_get_winsize
#define fl_get_win_origin        fl_get_winorigin
#define fl_get_win_geometry      fl_get_wingeometry
#define fl_initial_winposition   fl_pref_winposition

#define fl_get_display()           fl_display
#define FL_FormDisplay(form)       fl_display
#define FL_ObjectDisplay(object)   fl_display
#define FL_IS_CANVAS(o) (o->objclass == FL_CANVAS||o->objclass == FL_GLCANVAS)

/* the window an object belongs. For drawing */
#define FL_ObjWin(o) (FL_IS_CANVAS(o) ? fl_get_canvas_id(o): o->form->window)

FL_EXPORT Window fl_get_real_object_window(FL_OBJECT *);

#define FL_OBJECT_WID  FL_ObjWin

/*  all registerable events, including Client Message */
#define FL_ALL_EVENT  (KeyPressMask|KeyReleaseMask|      \
                      ButtonPressMask|ButtonReleaseMask|\
                      EnterWindowMask|LeaveWindowMask|    \
                      ButtonMotionMask|PointerMotionMask)

/* Timer related */

#define FL_TIMER_EVENT 0x40000000L


FL_EXPORT int fl_XNextEvent(XEvent *);
FL_EXPORT int fl_XPeekEvent(XEvent *);
FL_EXPORT int fl_XEventsQueued(int);
FL_EXPORT void fl_XPutBackEvent(XEvent *);
FL_EXPORT const XEvent *fl_last_event(void);

typedef int (*FL_APPEVENT_CB) (XEvent *, void *);
FL_EXPORT FL_APPEVENT_CB fl_set_event_callback(FL_APPEVENT_CB, void *);
FL_EXPORT FL_APPEVENT_CB fl_set_idle_callback(FL_APPEVENT_CB, void *);
FL_EXPORT long fl_addto_selected_xevent(Window, long);
FL_EXPORT long fl_remove_selected_xevent(Window, long);
#define fl_add_selected_xevent  fl_addto_selected_xevent
FL_EXPORT void fl_set_idle_delta(long);


/*
 * Group some WM stuff into a structure for easy maintainance
 */
enum
{
    FL_WM_SHIFT = 1, FL_WM_NORMAL = 2
};

typedef struct
{
    int rpx, rpy;		/* reparenting offset for full border */
    int trpx, trpy;		/* reparenting offset for transient   */
    int bw;			/* additional border                  */
    int rep_method;		/* 1 for shifting, 2 for normal       */
    unsigned pos_request;	/* USPOSITION or PPOSITION            */
}
FL_WM_STUFF;

FL_EXPORT FL_APPEVENT_CB fl_add_event_callback(Window, int,
					       FL_APPEVENT_CB, void *);

FL_EXPORT void fl_remove_event_callback(Window, int);
FL_EXPORT void fl_activate_event_callbacks(Window);

FL_EXPORT XEvent *fl_print_xevent_name(const char *, const XEvent *);


#define metakey_down(mask)     ((mask) & Mod1Mask)
#define shiftkey_down(mask)    ((mask) & ShiftMask)
#define controlkey_down(mask)  ((mask) & ControlMask)
#define button_down(mask)      (((mask) & Button1Mask) || \
                               ((mask) & Button2Mask) || \
			       ((mask) & Button3Mask))
#define fl_keypressed          fl_keysym_pressed

/****************** Resources ***************/

/* bool is int. */
typedef enum
{
    FL_NONE, FL_SHORT = 10, FL_BOOL, FL_INT, FL_LONG, FL_FLOAT, FL_STRING
}
FL_RTYPE;

typedef struct
{
    const char *res_name;	/* resource name                        */
    const char *res_class;	/* resource class                       */
    FL_RTYPE type;		/* FL_INT, FL_FLOAT, FL_BOOL,FL_STRING  */
    void *var;			/* address for the variable             */
    const char *defval;		/* default setting in string form       */
    int nbytes;			/* used only for strings                */
}
FL_RESOURCE;

#define FL_resource FL_RESOURCE

#define FL_CMD_OPT   XrmOptionDescRec

FL_EXPORT Display *fl_initialize(int *, char *[], const char *,
				 FL_CMD_OPT *, int);
FL_EXPORT void fl_finish(void);

FL_EXPORT const char *fl_get_resource(const char *, const char *,
				      FL_RTYPE, const char *, void *, int);
FL_EXPORT void fl_set_resource(const char *str, const char *val);

FL_EXPORT void fl_get_app_resources(FL_resource *, int n);
FL_EXPORT void fl_set_graphics_mode(int, int);
FL_EXPORT void fl_set_visualID(long);
FL_EXPORT int fl_keysym_pressed(KeySym);

#define buttonLabelSize  buttonFontSize
#define sliderLabelSize  sliderFontSize
#define inputLabelSize   inputFontSize

/* All Form control variables. Named closely as its resource name */
typedef struct
{
    float rgamma, ggamma, bgamma;
    int debug, sync;
    int depth, vclass, doubleBuffer;
    int ulPropWidth, ulThickness;	/* underline stuff       */
    int buttonFontSize;
    int sliderFontSize;
    int inputFontSize;
    int browserFontSize;
    int menuFontSize;
    int choiceFontSize;
    int labelFontSize;		/* all other labels fonts */
    int pupFontSize, pupFontStyle;	/* font for pop-up menus  */
    int privateColormap;
    int sharedColormap;
    int standardColormap;
    int scrollbarType;
    int backingStore;
    int coordUnit;
    int borderWidth;
    int safe;
    char *rgbfile;		/* where RGB file is     */
    char vname[24];
}
FL_IOPT;

#define FL_SBIT(n)   (1<<(n))

#define FL_PDButtonLabelSize FL_PDButtonFontSize
#define FL_PDSliderLabelSize FL_PDSliderFontSize
#define FL_PDInputLabelSize  FL_PDInputFontSize

/* program default masks */
enum
{
    FL_PDDepth = FL_SBIT(1),
    FL_PDClass = FL_SBIT(2),
    FL_PDDouble = FL_SBIT(3),
    FL_PDSync = FL_SBIT(4),
    FL_PDPrivateMap = FL_SBIT(5),
    FL_PDScrollbarType = FL_SBIT(6),
    FL_PDPupFontSize = FL_SBIT(7),
    FL_PDButtonFontSize = FL_SBIT(8),
    FL_PDInputFontSize = FL_SBIT(9),
    FL_PDSliderFontSize = FL_SBIT(10),
    FL_PDVisual = FL_SBIT(11),
    FL_PDULThickness = FL_SBIT(12),
    FL_PDULPropWidth = FL_SBIT(13),
    FL_PDBS = FL_SBIT(14),
    FL_PDCoordUnit = FL_SBIT(15),
    FL_PDDebug = FL_SBIT(16),
    FL_PDSharedMap = FL_SBIT(17),
    FL_PDStandardMap = FL_SBIT(18),
    FL_PDBorderWidth = FL_SBIT(19),
    FL_PDSafe = FL_SBIT(20),
    FL_PDMenuFontSize = FL_SBIT(21),
    FL_PDBrowserFontSize = FL_SBIT(22),
    FL_PDChoiceFontSize = FL_SBIT(23),
    FL_PDLabelFontSize = FL_SBIT(24)
};

#define FL_PDButtonLabel   FL_PDButtonLabelSize

FL_EXPORT void fl_set_defaults(unsigned long, FL_IOPT *);
FL_EXPORT void fl_set_tabstop(const char *s);
FL_EXPORT void fl_get_defaults(FL_IOPT *);
FL_EXPORT int fl_get_visual_depth(void);
FL_EXPORT const char *fl_vclass_name(int);
FL_EXPORT int fl_vclass_val(const char *);
FL_EXPORT void fl_set_ul_property(int, int);
FL_EXPORT void fl_set_clipping(FL_Coord, FL_Coord, FL_Coord, FL_Coord);
FL_EXPORT void fl_set_gc_clipping(GC, FL_Coord, FL_Coord, FL_Coord, FL_Coord);
FL_EXPORT void fl_unset_gc_clipping(GC);
FL_EXPORT void fl_set_clippings(FL_RECT *, int);
FL_EXPORT void fl_unset_clipping(void);

FL_EXPORT GC fl_textgc;

#define fl_set_text_clipping(a,b,c,d)   fl_set_gc_clipping(fl_textgc,a,b,c,d)
#define fl_unset_text_clipping() fl_unset_gc_clipping(fl_textgc)


#endif /* XBasic.h *} */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *   Object Class: Bitmap
 */
#ifndef FL_BITMAP_H
#define FL_BITMAP_H		/* { */

#define    FL_NORMAL_BITMAP      0

/***** Defaults *****/

#define FL_BITMAP_BOXTYPE	FL_NO_BOX
#define FL_BITMAP_COL1		FL_COL1		/* background of bitmap */
#define FL_BITMAP_COL2		FL_COL1		/* not used currently   */
#define FL_BITMAP_LCOL		FL_LCOL		/* foreground of bitmap */
#define FL_BITMAP_ALIGN		FL_ALIGN_BOTTOM

/***** Routines *****/
FL_EXPORT FL_OBJECT *fl_create_bitmap(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				      const char *);
FL_EXPORT FL_OBJECT *fl_add_bitmap(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, const char *);
FL_EXPORT void fl_set_bitmap_data(FL_OBJECT *, int, int, unsigned char *);
FL_EXPORT void fl_set_bitmap_file(FL_OBJECT *, const char *);
FL_EXPORT Pixmap fl_read_bitmapfile(Window, const char *,
				    unsigned *, unsigned *, int *, int *);

#define fl_create_from_bitmapdata(win, data, w, h)\
                   XCreateBitmapFromData(fl_get_display(), win, \
                   (char *)data, w, h)

/* for compatibility */
#define fl_set_bitmap_datafile fl_set_bitmap_file


/* PIXMAP stuff */

#define FL_NORMAL_PIXMAP   0

FL_EXPORT FL_OBJECT *fl_create_pixmap(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				      const char *);
FL_EXPORT FL_OBJECT *fl_add_pixmap(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				   const char *);

FL_EXPORT void fl_set_pixmap_data(FL_OBJECT *, char **);
FL_EXPORT void fl_set_pixmap_file(FL_OBJECT *, const char *);
FL_EXPORT void fl_set_pixmap_align(FL_OBJECT *, int, int, int);
FL_EXPORT void fl_set_pixmap_pixmap(FL_OBJECT *, Pixmap, Pixmap);
FL_EXPORT void fl_set_pixmap_colorcloseness(int, int, int);
FL_EXPORT void fl_free_pixmap_pixmap(FL_OBJECT *);
FL_EXPORT Pixmap fl_get_pixmap_pixmap(FL_OBJECT *, Pixmap *, Pixmap *);

FL_EXPORT Pixmap fl_read_pixmapfile(Window, const char *,
				    unsigned int *, unsigned int *,
				    Pixmap *, int *, int *, FL_COLOR);
FL_EXPORT Pixmap fl_create_from_pixmapdata(Window, char **,
					   unsigned int *, unsigned int *,
					   Pixmap *, int *, int *, FL_COLOR);
#define fl_free_pixmap(id)  do {if(id != None) XFreePixmap(fl_display, id);}while(0)

#endif /* BITMAP_H  } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_BOX_H
#define FL_BOX_H		/* { */


/* type already defined in Basic.h */


FL_EXPORT FL_OBJECT *fl_create_box(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				   const char *);

FL_EXPORT FL_OBJECT *fl_add_box(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				const char *);

#endif /* !def BOX_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *  Object class Browser
 */
#ifndef FL_BROWSER_H
#define FL_BROWSER_H		/* { */


/***** Types    *****/
typedef enum
{
    FL_NORMAL_BROWSER,
    FL_SELECT_BROWSER,
    FL_HOLD_BROWSER,
    FL_MULTI_BROWSER
}
FL_BROWSER_TYPE;

/***** Defaults *****/

#define FL_BROWSER_BOXTYPE	FL_DOWN_BOX
#define FL_BROWSER_COL1		FL_COL1
#define FL_BROWSER_COL2		FL_YELLOW
#define FL_BROWSER_LCOL		FL_LCOL
#define FL_BROWSER_ALIGN	FL_ALIGN_BOTTOM

/***** Others   *****/

#define FL_BROWSER_SLCOL	FL_COL1
#define FL_BROWSER_LINELENGTH	2048
#define FL_BROWSER_FONTSIZE     FL_SMALL_FONT

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_browser(int, FL_Coord, FL_Coord, FL_Coord,
				       FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_add_browser(int, FL_Coord, FL_Coord, FL_Coord,
				    FL_Coord, const char *);
FL_EXPORT void fl_clear_browser(FL_OBJECT *);
FL_EXPORT void fl_add_browser_line(FL_OBJECT *, const char *);
FL_EXPORT void fl_addto_browser(FL_OBJECT *, const char *);
FL_EXPORT void fl_addto_browser_chars(FL_OBJECT *, const char *);
#define fl_append_browser  fl_addto_browser_chars
FL_EXPORT void fl_insert_browser_line(FL_OBJECT *, int, const char *);
FL_EXPORT void fl_delete_browser_line(FL_OBJECT *, int);
FL_EXPORT void fl_replace_browser_line(FL_OBJECT *, int, const char *);
FL_EXPORT const char *fl_get_browser_line(FL_OBJECT *, int);
FL_EXPORT int fl_load_browser(FL_OBJECT *, const char *);

FL_EXPORT void fl_select_browser_line(FL_OBJECT *, int);
FL_EXPORT void fl_deselect_browser_line(FL_OBJECT *, int);
FL_EXPORT void fl_deselect_browser(FL_OBJECT *);
FL_EXPORT int fl_isselected_browser_line(FL_OBJECT *, int);

FL_EXPORT int fl_get_browser_topline(FL_OBJECT *);
FL_EXPORT int fl_get_browser(FL_OBJECT *);
FL_EXPORT int fl_get_browser_maxline(FL_OBJECT *);
FL_EXPORT int fl_get_browser_screenlines(FL_OBJECT *);

FL_EXPORT void fl_set_browser_topline(FL_OBJECT *, int);
FL_EXPORT void fl_set_browser_fontsize(FL_OBJECT *, int);
FL_EXPORT void fl_set_browser_fontstyle(FL_OBJECT *, int);
FL_EXPORT void fl_set_browser_specialkey(FL_OBJECT *, int);
FL_EXPORT void fl_set_browser_vscrollbar(FL_OBJECT *, int);
FL_EXPORT void fl_set_browser_hscrollbar(FL_OBJECT *, int);
FL_EXPORT void fl_set_browser_line_selectable(FL_OBJECT *, int, int);
FL_EXPORT void fl_get_browser_dimension(FL_OBJECT *, FL_Coord *, FL_Coord *,
					FL_Coord *, FL_Coord *);
FL_EXPORT void fl_set_browser_dblclick_callback(FL_OBJECT *,
						FL_CALLBACKPTR, long);

FL_EXPORT void fl_set_browser_xoffset(FL_OBJECT *, FL_Coord);
FL_EXPORT FL_Coord fl_get_browser_xoffset(FL_OBJECT *);
FL_EXPORT void fl_set_browser_scrollbarsize(FL_OBJECT *, int, int);
FL_EXPORT void fl_show_browser_line(FL_OBJECT *, int);

#endif /* BROWSER_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 * All Buttons: regular button, light button and round button
 *
 */

#ifndef FL_BUTTON_H
#define FL_BUTTON_H		/* { */


typedef enum
{
    FL_NORMAL_BUTTON,
    FL_PUSH_BUTTON,
    FL_RADIO_BUTTON,
    FL_HIDDEN_BUTTON,
    FL_TOUCH_BUTTON,
    FL_INOUT_BUTTON,
    FL_RETURN_BUTTON,
    FL_HIDDEN_RET_BUTTON,
    FL_MENU_BUTTON
}
FL_BUTTON_TYPE;

#define FL_TOGGLE_BUTTON    FL_PUSH_BUTTON

typedef struct
{
    Pixmap pixmap, mask;
    unsigned bits_w, bits_h;
    int val;			/* whether on */
    int mousebut;		/* mouse button that caused the push     */
    int timdel;			/* time since last touch (TOUCH buttons) */
    int event;			/* what event triggers redraw            */
    long cspecl;		/* reserved for class specfic stuff      */
    void *cspecv;		/* misc. things                          */
    char *filename;
}
FL_BUTTON_SPEC;

#define FL_BUTTON_STRUCT FL_BUTTON_SPEC

typedef void (*FL_DrawButton) (FL_OBJECT *);
typedef void (*FL_CleanupButton) (FL_BUTTON_STRUCT *);

#define FL_DRAWBUTTON      FL_DrawButton
#define FL_CLEANUPBUTTON   FL_CleanupButton

/*
 *  normal button default
 */
#define FL_BUTTON_BOXTYPE	FL_UP_BOX
#define FL_BUTTON_COL1		FL_COL1
#define FL_BUTTON_COL2		FL_COL1
#define FL_BUTTON_LCOL		FL_LCOL
#define FL_BUTTON_ALIGN		FL_ALIGN_CENTER
#define FL_BUTTON_MCOL1		FL_MCOL
#define FL_BUTTON_MCOL2		FL_MCOL
#define FL_BUTTON_BW		FL_BOUND_WIDTH

/*
 *  light button defaults
 */
#define FL_LIGHTBUTTON_BOXTYPE	FL_UP_BOX
#define FL_LIGHTBUTTON_COL1	FL_COL1
#define FL_LIGHTBUTTON_COL2	FL_YELLOW
#define FL_LIGHTBUTTON_LCOL	FL_LCOL
#define FL_LIGHTBUTTON_ALIGN	FL_ALIGN_CENTER
#define FL_LIGHTBUTTON_TOPCOL	FL_COL1
#define FL_LIGHTBUTTON_MCOL	FL_MCOL
#define FL_LIGHTBUTTON_MINSIZE	(FL_Coord)12

/** round button defaults ***/

#define FL_ROUNDBUTTON_BOXTYPE FL_NO_BOX
#define FL_ROUNDBUTTON_COL1	 FL_MCOL
#define FL_ROUNDBUTTON_COL2	 FL_YELLOW
#define FL_ROUNDBUTTON_LCOL	 FL_LCOL
#define FL_ROUNDBUTTON_ALIGN	 FL_ALIGN_CENTER
#define FL_ROUNDBUTTON_TOPCOL	 FL_COL1
#define FL_ROUNDBUTTON_MCOL	 FL_MCOL

/* round3d button defaults   */

#define FL_ROUND3DBUTTON_BOXTYPE FL_NO_BOX
#define FL_ROUND3DBUTTON_COL1	 FL_COL1
#define FL_ROUND3DBUTTON_COL2	 FL_BLACK
#define FL_ROUND3DBUTTON_LCOL	 FL_LCOL
#define FL_ROUND3DBUTTON_ALIGN	 FL_ALIGN_CENTER
#define FL_ROUND3DBUTTON_TOPCOL	 FL_COL1
#define FL_ROUND3DBUTTON_MCOL	 FL_MCOL

/** check button defaults ***/

#define FL_CHECKBUTTON_BOXTYPE	FL_NO_BOX
#define FL_CHECKBUTTON_COL1	FL_COL1
#define FL_CHECKBUTTON_COL2	FL_YELLOW
#define FL_CHECKBUTTON_LCOL	FL_LCOL
#define FL_CHECKBUTTON_ALIGN	FL_ALIGN_CENTER

#define FL_CHECKBUTTON_TOPCOL	FL_COL1
#define FL_CHECKBUTTON_MCOL	FL_MCOL

/** bitmap button defaults **/
#define FL_BITMAPBUTTON_BOXTYPE	FL_UP_BOX
#define FL_BITMAPBUTTON_COL1	FL_COL1		/* bitmap background  */
#define FL_BITMAPBUTTON_COL2	FL_BLUE		/* "focus" color       */
#define FL_BITMAPBUTTON_LCOL	FL_LCOL		/* bitmap foreground   */
#define FL_BITMAPBUTTON_ALIGN	FL_ALIGN_BOTTOM

/** bitmap button defaults **/
#define FL_PIXMAPBUTTON_BOXTYPE	FL_UP_BOX
#define FL_PIXMAPBUTTON_COL1	FL_COL1		/* box col    */
#define FL_PIXMAPBUTTON_COL2	FL_YELLOW	/* bound rect */
#define FL_PIXMAPBUTTON_LCOL	FL_LCOL
#define FL_PIXMAPBUTTON_ALIGN	FL_ALIGN_BOTTOM

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_button(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				      const char *);
FL_EXPORT FL_OBJECT *fl_create_roundbutton(int, FL_Coord, FL_Coord, FL_Coord,
					   FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_create_round3dbutton(int, FL_Coord, FL_Coord, FL_Coord,
					     FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_create_lightbutton(int, FL_Coord, FL_Coord, FL_Coord,
					   FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_create_checkbutton(int, FL_Coord, FL_Coord, FL_Coord,
					   FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_create_bitmapbutton(int, FL_Coord, FL_Coord, FL_Coord,
					    FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_create_pixmapbutton(int, FL_Coord, FL_Coord, FL_Coord,
					    FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_create_scrollbutton(int, FL_Coord, FL_Coord, FL_Coord,
					    FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_roundbutton(int, FL_Coord, FL_Coord,
					FL_Coord, FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_add_round3dbutton(int, FL_Coord, FL_Coord,
					  FL_Coord, FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_add_lightbutton(int, FL_Coord, FL_Coord,
					FL_Coord, FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_add_checkbutton(int, FL_Coord, FL_Coord,
					FL_Coord, FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_add_button(int, FL_Coord, FL_Coord, FL_Coord,
				   FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_bitmapbutton(int, FL_Coord, FL_Coord, FL_Coord,
					 FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_add_scrollbutton(int, FL_Coord, FL_Coord, FL_Coord,
					 FL_Coord, const char *);

FL_EXPORT void fl_set_bitmapbutton_file(FL_OBJECT *, const char *);
FL_EXPORT void fl_set_bitmapbutton_data(FL_OBJECT *, int, int, unsigned char *);

#define fl_set_bitmapbutton_datafile  fl_set_bitmapbutton_file

FL_EXPORT FL_OBJECT *fl_add_pixmapbutton(int, FL_Coord, FL_Coord, FL_Coord,
					 FL_Coord, const char *);

#define fl_set_pixmapbutton_data      fl_set_pixmap_data
#define fl_set_pixmapbutton_file      fl_set_pixmap_file
#define fl_set_pixmapbutton_pixmap    fl_set_pixmap_pixmap
#define fl_get_pixmapbutton_pixmap    fl_get_pixmap_pixmap
#define fl_set_pixmapbutton_align     fl_set_pixmap_align
#define fl_free_pixmapbutton_pixmap   fl_free_pixmap_pixmap
#define fl_set_pixmapbutton_datafile  fl_set_pixmapbutton_file
FL_EXPORT void fl_set_pixmapbutton_focus_outline(FL_OBJECT *, int);

FL_EXPORT int fl_get_button(FL_OBJECT *);
FL_EXPORT void fl_set_button(FL_OBJECT *, int);
FL_EXPORT int fl_get_button_numb(FL_OBJECT *);

#define fl_set_button_shortcut  fl_set_object_shortcut

FL_EXPORT FL_OBJECT *fl_create_generic_button(int, int, FL_Coord, FL_Coord,
					  FL_Coord, FL_Coord, const char *);
FL_EXPORT void fl_add_button_class(int, FL_DRAWBUTTON, FL_CLEANUPBUTTON);


#endif /* Button.h  } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 * Header for FL_CANVAS
 *
 */

#ifndef FL_CANVAS_H_
#define FL_CANVAS_H		/*************{*/


typedef enum
{
    FL_NORMAL_CANVAS,
    FL_SCROLLED_CANVAS
} FL_CANVAS_TYPE;


typedef int (*FL_HANDLE_CANVAS) (FL_OBJECT * ob,
				 Window,
				 int, int,
				 XEvent *, void *);

typedef int (*FL_MODIFY_CANVAS_PROP) (FL_OBJECT *);

/******************** Default *********************/

#define FL_CANVAS_BOXTYPE   FL_DOWN_BOX	/* really the decoration frame */
#define FL_CANVAS_ALIGN     FL_ALIGN_TOP


/************ Interfaces    ************************/


FL_EXPORT FL_OBJECT *fl_create_generic_canvas(int, int, FL_Coord, FL_Coord,
					  FL_Coord, FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_canvas(int, FL_Coord, FL_Coord, FL_Coord,
				   FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_create_canvas(int, FL_Coord, FL_Coord, FL_Coord,
				      FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_create_mesacanvas(int, FL_Coord, FL_Coord, FL_Coord,
					  FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_mesacanvas(int, FL_Coord, FL_Coord, FL_Coord,
				       FL_Coord, const char *);


/* backward compatibility */
#define fl_set_canvas_decoration fl_set_object_boxtype

FL_EXPORT void fl_set_canvas_colormap(FL_OBJECT *, Colormap);
FL_EXPORT void fl_set_canvas_visual(FL_OBJECT *, Visual *);
FL_EXPORT void fl_set_canvas_depth(FL_OBJECT *, int);
FL_EXPORT void fl_set_canvas_attributes(FL_OBJECT *, unsigned,
					XSetWindowAttributes *);

FL_EXPORT FL_HANDLE_CANVAS fl_add_canvas_handler(FL_OBJECT *, int,
						 FL_HANDLE_CANVAS, void *);

FL_EXPORT Window fl_get_canvas_id(FL_OBJECT *);
FL_EXPORT Colormap fl_get_canvas_colormap(FL_OBJECT *);
FL_EXPORT int fl_get_canvas_depth(FL_OBJECT *);
FL_EXPORT void fl_remove_canvas_handler(FL_OBJECT *, int, FL_HANDLE_CANVAS);
FL_EXPORT void fl_hide_canvas(FL_OBJECT *);	/* internal use only */
FL_EXPORT void fl_canvas_yield_to_shortcut(FL_OBJECT *, int);
FL_EXPORT void fl_share_canvas_colormap(FL_OBJECT *, Colormap);
FL_EXPORT void fl_modify_canvas_prop(FL_OBJECT *,
				     FL_MODIFY_CANVAS_PROP,
				     FL_MODIFY_CANVAS_PROP,
				     FL_MODIFY_CANVAS_PROP);

/* OpenGL canvases */
FL_EXPORT FL_OBJECT *fl_create_glcanvas(int, FL_Coord, FL_Coord, FL_Coord,
					FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_glcanvas(int, FL_Coord, FL_Coord, FL_Coord,
				     FL_Coord, const char *);

FL_EXPORT void fl_set_glcanvas_defaults(const int *);
FL_EXPORT void fl_get_glcanvas_defaults(int *);
FL_EXPORT void fl_set_glcanvas_attributes(FL_OBJECT *, const int *);
FL_EXPORT void fl_get_glcanvas_attributes(FL_OBJECT *, int *);
FL_EXPORT void fl_set_glcanvas_direct(FL_OBJECT *, int);
FL_EXPORT void fl_activate_glcanvas(FL_OBJECT *);
FL_EXPORT XVisualInfo *fl_get_glcanvas_xvisualinfo(FL_OBJECT *);


#if defined(__GLX_glx_h__) || defined(GLX_H)
FL_EXPORT GLXContext fl_get_glcanvas_context(FL_OBJECT * ob);
FL_EXPORT Window fl_glwincreate(int *, GLXContext *, int, int);
FL_EXPORT Window fl_glwinopen(int *, GLXContext *, int, int);
#endif

#endif /**** ! FL_CANVAS_H *****}**/
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 * Object Class: Chart
 *
 */
#ifndef FL_CHART_H
#define FL_CHART_H		/* { */


typedef enum
{
    FL_BAR_CHART,
    FL_HORBAR_CHART,
    FL_LINE_CHART,
    FL_FILL_CHART,
    FL_SPIKE_CHART,
    FL_PIE_CHART,
    FL_SPECIALPIE_CHART
}
FL_CHART_TYPE;

#define FL_FILLED_CHART  FL_FILL_CHART	/* compatibility */

/***** Defaults *****/

#define FL_CHART_BOXTYPE	FL_BORDER_BOX
#define FL_CHART_COL1		FL_COL1
#define FL_CHART_LCOL		FL_LCOL
#define FL_CHART_ALIGN		FL_ALIGN_BOTTOM

/***** Others   *****/

#define FL_CHART_MAX		512

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_chart(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				     const char *);
FL_EXPORT FL_OBJECT *fl_add_chart(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				  const char *);

FL_EXPORT void fl_clear_chart(FL_OBJECT *);
FL_EXPORT void fl_add_chart_value(FL_OBJECT *, double, const char *, int);
FL_EXPORT void fl_insert_chart_value(FL_OBJECT *, int, double, const char *, int);
FL_EXPORT void fl_replace_chart_value(FL_OBJECT *, int, double, const char *, int);
FL_EXPORT void fl_set_chart_bounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_set_chart_maxnumb(FL_OBJECT *, int);
FL_EXPORT void fl_set_chart_autosize(FL_OBJECT *, int);
FL_EXPORT void fl_set_chart_lstyle(FL_OBJECT *, int);
FL_EXPORT void fl_set_chart_lsize(FL_OBJECT *, int);
FL_EXPORT void fl_set_chart_lcolor(FL_OBJECT *, FL_COLOR);
#define fl_set_chart_lcol   fl_set_chart_lcolor

#endif /* !def CHART_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_CHOICE_H
#define FL_CHOICE_H		/* { */


typedef enum
{
    FL_NORMAL_CHOICE,
    FL_NORMAL_CHOICE2,
    FL_DROPLIST_CHOICE
}
FL_CHOICE_TYPE;

#define  FL_SIMPLE_CHOICE  FL_NORMAL_CHOICE

/***** Defaults *****/

#define FL_CHOICE_BOXTYPE	FL_ROUNDED_BOX
#define FL_CHOICE_COL1		FL_COL1
#define FL_CHOICE_COL2		FL_LCOL
#define FL_CHOICE_LCOL		FL_LCOL
#define FL_CHOICE_ALIGN		FL_ALIGN_LEFT

/***** Others   *****/

#define FL_CHOICE_MCOL		FL_MCOL
#define FL_CHOICE_MAXITEMS	80

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_choice(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				      const char *);

FL_EXPORT FL_OBJECT *fl_add_choice(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, const char *);
FL_EXPORT void fl_clear_choice(FL_OBJECT *);
FL_EXPORT int fl_addto_choice(FL_OBJECT *, const char *);
FL_EXPORT void fl_replace_choice(FL_OBJECT *, int, const char *);
FL_EXPORT void fl_delete_choice(FL_OBJECT *, int);
FL_EXPORT void fl_set_choice(FL_OBJECT *, int);
FL_EXPORT void fl_set_choice_text(FL_OBJECT *, const char *);
FL_EXPORT int fl_get_choice(FL_OBJECT *);
FL_EXPORT const char *fl_get_choice_item_text(FL_OBJECT *, int);
FL_EXPORT int fl_get_choice_maxitems(FL_OBJECT *);
FL_EXPORT const char *fl_get_choice_text(FL_OBJECT *);
FL_EXPORT void fl_set_choice_fontsize(FL_OBJECT *, int);
FL_EXPORT void fl_set_choice_fontstyle(FL_OBJECT *, int);
FL_EXPORT void fl_set_choice_align(FL_OBJECT *, int);
FL_EXPORT void fl_set_choice_item_mode(FL_OBJECT *, int, unsigned);
FL_EXPORT void fl_set_choice_item_shortcut(FL_OBJECT *, int, const char *);
FL_EXPORT int fl_set_choice_entries(FL_OBJECT *, FL_PUP_ENTRY *);

#endif /* !def CHOICE_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 * prototypes for clipboard stuff
 *
 */
#ifndef FL_CLIPBD_H
#define FL_CLIPBD_H

typedef Atom FL_CPTYPE;

typedef int (*FL_LOSE_SELECTION_CB)(FL_OBJECT *ob, long type);
typedef int (*FL_SELECTION_CB)(FL_OBJECT *ob, long type,
			     const void *data, long size);

#define FL_SELECTION_CALLBACK        FL_SELECTION_CB
#define FL_LOSE_SELECTION_CALLBACK   FL_LOSE_SELECTION_CB

FL_EXPORT int fl_stuff_clipboard(FL_OBJECT *ob, long type,
			      const void *data, long size,
			      FL_LOSE_SELECTION_CB cb);

FL_EXPORT int fl_request_clipboard(FL_OBJECT *ob, long type,
				FL_SELECTION_CB callback);

#endif /* !def CLIPBD_H */

/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_CLOCK_H
#define FL_CLOCK_H		/* { */


enum
{
    FL_ANALOG_CLOCK,
    FL_DIGITAL_CLOCK
};

#define FL_CLOCK_BOXTYPE   FL_UP_BOX
#define FL_CLOCK_COL1      FL_INACTIVE_COL
#define FL_CLOCK_COL2      FL_BOTTOM_BCOL
#define FL_CLOCK_LCOL      FL_BLACK
#define FL_CLOCK_ALIGN     FL_ALIGN_BOTTOM

#define FL_CLOCK_TOPCOL  FL_COL1

FL_EXPORT FL_OBJECT *fl_create_clock(int, FL_Coord, FL_Coord, FL_Coord,
				     FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_clock(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, const char *);
FL_EXPORT void fl_get_clock(FL_OBJECT *, int *, int *, int *);
FL_EXPORT long fl_set_clock_adjustment(FL_OBJECT *, long);
FL_EXPORT void fl_set_clock_ampm(FL_OBJECT *, int);

#endif /* !def CLOCK_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */

#ifndef FL_COUNTER_H
#define FL_COUNTER_H		/* { */


typedef enum
{
    FL_NORMAL_COUNTER,
    FL_SIMPLE_COUNTER
}
FL_COUNTER_TYPE;

/***** Defaults *****/

#define FL_COUNTER_BOXTYPE	FL_UP_BOX
#define FL_COUNTER_COL1		FL_COL1
#define FL_COUNTER_COL2		FL_BLUE		/* ct label     */
#define FL_COUNTER_LCOL		FL_LCOL		/* ct reporting */
#define FL_COUNTER_ALIGN	FL_ALIGN_BOTTOM

/***** Others   *****/

#define FL_COUNTER_BW		(FL_BOUND_WIDTH-1)

/***** Routines *****/
FL_EXPORT FL_OBJECT *fl_create_counter(int, FL_Coord, FL_Coord, FL_Coord,
				       FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_counter(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				    const char *);

FL_EXPORT void fl_set_counter_value(FL_OBJECT *, double);
FL_EXPORT void fl_set_counter_bounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_set_counter_step(FL_OBJECT *, double, double);
FL_EXPORT void fl_set_counter_precision(FL_OBJECT *, int);
FL_EXPORT void fl_set_counter_return(FL_OBJECT *, int);
FL_EXPORT double fl_get_counter_value(FL_OBJECT *);
FL_EXPORT void fl_get_counter_bounds(FL_OBJECT *, double *, double *);
FL_EXPORT void fl_get_counter_step(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_set_counter_filter(FL_OBJECT *,
				 const char *(*)(FL_OBJECT *, double, int));


#endif /* !def  COUNTER_H } */

/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 * Cursor defs and prototypes
 *
 */

#ifndef FL_CURSOR_H
#define FL_CURSOR_H		/* { */

#include <X11/cursorfont.h>

#define FL_DEFAULT_CURSOR     -1
#define FL_INVISIBLE_CURSOR   -2

#ifndef XC_invisible
#define XC_invisible   FL_INVISIBLE_CURSOR
#endif

FL_EXPORT void fl_set_cursor(Window, int);
FL_EXPORT void fl_set_cursor_color(int, FL_COLOR, FL_COLOR);
FL_EXPORT int fl_create_bitmap_cursor(const char *, const char *,
				      int, int, int, int);
FL_EXPORT int fl_create_animated_cursor(int *, int);

FL_EXPORT Cursor fl_get_cursor_byname(int);

#define fl_reset_cursor(win) fl_set_cursor(win, FL_DEFAULT_CURSOR);

#endif /* cursor.h   *} */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 */
#ifndef FL_DIAL_H
#define FL_DIAL_H		/* { */


typedef enum
{
    FL_NORMAL_DIAL,
    FL_LINE_DIAL,
    FL_FILL_DIAL
}
FL_DIAL_TYPE;

enum
{
    FL_DIAL_CW, FL_DIAL_CCW
};

/***** Defaults *****/

#define FL_DIAL_BOXTYPE		FL_FLAT_BOX
#define FL_DIAL_COL1		FL_COL1
#define FL_DIAL_COL2		FL_RIGHT_BCOL
#define FL_DIAL_LCOL		FL_LCOL
#define FL_DIAL_ALIGN		FL_ALIGN_BOTTOM

/***** Others   *****/

#define FL_DIAL_TOPCOL		FL_COL1

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_dial(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				    const char *);
FL_EXPORT FL_OBJECT *fl_add_dial(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				 const char *);

FL_EXPORT void fl_set_dial_value(FL_OBJECT *, double);
FL_EXPORT double fl_get_dial_value(FL_OBJECT *);
FL_EXPORT void fl_set_dial_bounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_dial_bounds(FL_OBJECT *, double *, double *);

FL_EXPORT void fl_set_dial_step(FL_OBJECT *, double);
FL_EXPORT void fl_set_dial_return(FL_OBJECT *, int);
FL_EXPORT void fl_set_dial_angles(FL_OBJECT *, double, double);
FL_EXPORT void fl_set_dial_cross(FL_OBJECT *, int);
FL_EXPORT void fl_set_dial_direction(FL_OBJECT *, int);

#endif /* !def DIAL_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *  Convenience functions to read a directory
 */

#ifndef FL_FILESYS_H
#define FL_FILESYS_H		/* { */

/*  File types */
enum
{
    FT_FILE, FT_DIR, FT_LINK, FT_SOCK,
    FT_FIFO, FT_BLK, FT_CHR, FT_OTHER
};

typedef struct
{
    char *name;			/* entry name             */
    int type;			/* FILE_TYPE              */
    long dl_mtime;		/* file modification time */
    unsigned long dl_size;	/* file size in bytes     */
    long filler[3];		/* reserved               */
}
FL_Dirlist;

typedef int (*FL_DIRLIST_FILTER) (const char *, int);

/* read dir with pattern filtering. All dirs read might be cached.
 * must not change dirlist in anyway.
 */
FL_EXPORT const FL_Dirlist *fl_get_dirlist(const char *,	/* dir */
					   const char *,	/* pat */
					   int *,	/* nfiles */
					   int);	/* rescan */
enum
{
    FL_ALPHASORT = 1,
    FL_RALPHASORT,
    FL_MTIMESORT,
    FL_RMTIMESORT,
    FL_SIZESORT,
    FL_RSIZESORT
};

FL_EXPORT FL_DIRLIST_FILTER fl_set_dirlist_filter(FL_DIRLIST_FILTER);
FL_EXPORT void fl_set_dirlist_sort(int);

FL_EXPORT void fl_free_dirlist(FL_Dirlist *);

/* Free all directory caches */
FL_EXPORT void fl_free_all_dirlist(void);


/*FL_EXPORT int fl_is_valid_dir(const char *);*/
/*FL_EXPORT unsigned long fl_fmtime(const char *);*/
FL_EXPORT char *fl_fix_dirname(char *);

#endif /* ! def FL_FILESYS_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 */

#ifndef FLPS_H
#define FLPS_H

/* postscript stuff */

enum 
{
  FLPS_AUTO, FLPS_LANDSCAPE, FLPS_PORTRAIT
};

enum
{
  FLPS_BW = -1, FLPS_GRAYSCALE, FLPS_COLOR
};

#define FLPS_MEMBERS                                \
             int ps_color;                          \
             int orientation;                       \
             int auto_fit;                          \
             int drawbox;                           \
             int eps;                               \
             float xdpi, ydpi;                      \
             float paper_w, paper_h;                \
             float gamma

/* configuration infomation */
typedef struct { FLPS_MEMBERS; } FLPS_CONTROL;

FL_EXPORT FLPS_CONTROL *flps_init(void);
FL_EXPORT int fl_object_ps_dump(FL_OBJECT *, const char *);

#endif
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */

#ifndef FL_FRAME_H
#define FL_FRAME_H		/* { */

/* types of frames */
enum
{
    FL_NO_FRAME,
    FL_UP_FRAME,
    FL_DOWN_FRAME,
    FL_BORDER_FRAME,
    FL_SHADOW_FRAME,
    FL_ENGRAVED_FRAME,
    FL_ROUNDED_FRAME,
    FL_EMBOSSED_FRAME,
    FL_OVAL_FRAME
};

#define FL_FRAME_COL1   FL_BLACK   /* border color     */ 
#define FL_FRAME_COL2   FL_COL1    /* label background */
#define FL_FRAME_LCOL   FL_BLACK   /* label color      */

FL_EXPORT FL_OBJECT *fl_create_frame(int, FL_Coord, FL_Coord, 
                                  FL_Coord, FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_frame(int,FL_Coord,FL_Coord,
                             FL_Coord,FL_Coord, const char *);

/* labeld frame */
FL_EXPORT FL_OBJECT *fl_create_labelframe(int, FL_Coord, FL_Coord, 
                                  FL_Coord, FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_labelframe(int,FL_Coord,FL_Coord,
                             FL_Coord,FL_Coord, const char *);
#endif /* !def FRAME_H } */

/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *  Object Class: Free
 */
#ifndef FL_FREE_H
#define FL_FREE_H		/* { */


typedef enum
{
    FL_NORMAL_FREE,
    FL_INACTIVE_FREE,
    FL_INPUT_FREE,
    FL_CONTINUOUS_FREE,
    FL_ALL_FREE
}
FL_FREE_TYPE;

#define FL_SLEEPING_FREE  FL_INACTIVE_FREE

FL_EXPORT FL_OBJECT *fl_create_free(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				    const char *, FL_HANDLEPTR);
FL_EXPORT FL_OBJECT *fl_add_free(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				 const char *, FL_HANDLEPTR);
#endif /* !def FREE_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 */
#ifndef FL_GOODIES_H
#define FL_GOODIES_H		/* { */

/***** Resources and misc. goodie routines ******/
#define FLAlertDismissLabel     "flAlert.dismiss.label"
#define FLAlertTitle            "flAlert.title"

#define FLQuestionYesLabel      "flQuestion.yes.label"
#define FLQuestionNoLabel       "flQuestion.no.label"
#define FLQuestionTitle         "flQuestion.title"

#define FLOKLabel               "flInput.ok.label"
#define FLInputClearLabel       "flInput.clear.label"
#define FLInputCancelLabel      "flInput.cancel.label"
#define FLInputTitle            "flInput.title"

#define FLChoiceTitle           "flChoice.title"

FL_EXPORT void fl_set_goodies_font(int, int);

/*********** messages and questions **************/

FL_EXPORT void fl_show_message(const char *, const char *, const char *);
FL_EXPORT void fl_show_messages(const char *);
FL_EXPORT void fl_hide_message(void);

FL_EXPORT int fl_show_question(const char *, int);
FL_EXPORT void fl_hide_question(void);

FL_EXPORT void fl_show_alert(const char *, const char *, const char *, int);
FL_EXPORT void fl_hide_alert(void);

FL_EXPORT const char *fl_show_input(const char *, const char *);
FL_EXPORT void fl_hide_input(void);
FL_EXPORT const char *fl_show_simple_input(const char *, const char *);

FL_EXPORT int fl_show_colormap(int);

/********* choices *****************/
FL_EXPORT int fl_show_choices(const char *, int,
			      const char *, const char *, const char *, int);

FL_EXPORT int fl_show_choice(const char *, const char *, const char *, int,
			     const char *, const char *, const char *, int);
FL_EXPORT void fl_hide_choice(void);

FL_EXPORT void fl_set_choice_shortcut(const char *, const char *, const char *);
#define fl_set_choices_shortcut fl_set_choice_shortcut

/************ one liner ***************/

FL_EXPORT void fl_show_oneliner(const char *, FL_Coord, FL_Coord);
FL_EXPORT void fl_hide_oneliner(void);
FL_EXPORT void fl_set_oneliner_font(int, int);
FL_EXPORT void fl_set_oneliner_color(FL_COLOR, FL_COLOR);

/************* command log **************/

typedef struct
{
    FL_FORM *form;
    FL_OBJECT *browser;
    FL_OBJECT *close_browser;
    FL_OBJECT *clear_browser;
}
FD_CMDLOG;

#ifdef FL_WIN32
#define FL_PID_T HANDLE
#else
#define FL_PID_T long
#endif

FL_EXPORT FL_PID_T fl_exe_command(const char *, int);
FL_EXPORT int fl_end_command(FL_PID_T);
FL_EXPORT int fl_check_command(FL_PID_T);
FL_EXPORT int fl_end_all_command(void);
FL_EXPORT void fl_show_command_log(int);
FL_EXPORT void fl_hide_command_log(void);
FL_EXPORT void fl_clear_command_log(void);
FL_EXPORT void fl_addto_command_log(const char *s);
FL_EXPORT void fl_set_command_log_position(int, int);
FL_EXPORT FD_CMDLOG *fl_get_command_log_fdstruct(void);

/* aliases */
#define fl_open_command    fl_exe_command
#define fl_close_command   fl_end_command

/******* file selector *****************/

#define FL_MAX_FSELECTOR  6

typedef struct
{
    FL_FORM *fselect;
    FL_OBJECT *browser, *input, *prompt, *resbutt;
    FL_OBJECT *patbutt, *dirbutt, *cancel, *ready;
    FL_OBJECT *dirlabel, *patlabel;
    FL_OBJECT *appbutt[3];
}
FD_FSELECTOR;

FL_EXPORT int fl_use_fselector(int);
FL_EXPORT const char *fl_show_fselector(const char *, const char *,
					const char *, const char *);
FL_EXPORT void fl_hide_fselector(void);

FL_EXPORT void fl_set_fselector_fontsize(int);
FL_EXPORT void fl_set_fselector_fontstyle(int);
FL_EXPORT void fl_set_fselector_placement(int);
FL_EXPORT void fl_set_fselector_border(int);

#define fl_set_fselector_transient(b)   \
                     fl_set_fselector_border((b)?FL_TRANSIENT:FL_FULLBORDER)

FL_EXPORT void fl_set_fselector_callback(int (*)(const char *, void *), void *);
FL_EXPORT const char *fl_get_filename(void);
FL_EXPORT const char *fl_get_directory(void);
FL_EXPORT const char *fl_get_pattern(void);
FL_EXPORT int fl_set_directory(const char *);
FL_EXPORT void fl_set_pattern(const char *);
FL_EXPORT void fl_refresh_fselector(void);
FL_EXPORT void fl_add_fselector_appbutton(const char *, void (*)(void *), void *);
FL_EXPORT void fl_remove_fselector_appbutton(const char *);
FL_EXPORT void fl_disable_fselector_cache(int);
FL_EXPORT void fl_invalidate_fselector_cache(void);
FL_EXPORT FL_FORM *fl_get_fselector_form(void);
FL_EXPORT FD_FSELECTOR *fl_get_fselector_fdstruct(void);
FL_EXPORT void fl_hide_fselector(void);

FL_EXPORT void fl_set_fselector_filetype_marker(int dir, int fifo,
					      int sock, int cdev, int bdev);

#define fl_show_file_selector     fl_show_fselector
#define fl_set_fselector_cb       fl_set_fselector_callback
#define fl_set_fselector_title(s) fl_set_form_title(fl_get_fselector_form(),s)

#endif /* } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_INPUT_H
#define FL_INPUT_H		/* { */


/***** Types    *****/

typedef enum
{
    FL_NORMAL_INPUT,
    FL_FLOAT_INPUT,
    FL_INT_INPUT,
    FL_DATE_INPUT,
    FL_MULTILINE_INPUT,
    FL_HIDDEN_INPUT,
    FL_SECRET_INPUT
}
FL_INPUT_TYPE;

/* for date input */
enum
{
    FL_INPUT_MMDD, FL_INPUT_DDMM
};

/***** Defaults *****/

#define FL_INPUT_BOXTYPE	FL_DOWN_BOX
#define FL_INPUT_COL1		FL_COL1
#define FL_INPUT_COL2		FL_MCOL
#define FL_INPUT_LCOL		FL_LCOL
#define FL_INPUT_ALIGN		FL_ALIGN_LEFT

/***** Others   *****/

#define FL_INPUT_TCOL		FL_LCOL
#define FL_INPUT_CCOL		FL_BLUE

#define FL_RINGBELL             (1<<4)

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_input(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				     const char *);

FL_EXPORT FL_OBJECT *fl_add_input(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, const char *);

FL_EXPORT void fl_set_input(FL_OBJECT *, const char *);
FL_EXPORT void fl_set_input_return(FL_OBJECT *, int);
FL_EXPORT void fl_set_input_color(FL_OBJECT *, int, int);
FL_EXPORT void fl_set_input_scroll(FL_OBJECT *, int);
FL_EXPORT void fl_set_input_cursorpos(FL_OBJECT *, int, int);
FL_EXPORT void fl_set_input_selected(FL_OBJECT *, int);
FL_EXPORT void fl_set_input_selected_range(FL_OBJECT *, int, int);
FL_EXPORT const char *fl_get_input_selected_range(FL_OBJECT *, int *, int *);
FL_EXPORT void fl_set_input_maxchars(FL_OBJECT *, int);
FL_EXPORT void fl_set_input_format(FL_OBJECT *, int, int);
FL_EXPORT void fl_set_input_hscrollbar(FL_OBJECT *, int);
FL_EXPORT void fl_set_input_vscrollbar(FL_OBJECT *, int);
FL_EXPORT void fl_set_input_topline(FL_OBJECT *, int);
FL_EXPORT void fl_set_input_scrollbarsize(FL_OBJECT *, int, int);
FL_EXPORT void fl_set_input_xoffset(FL_OBJECT *, int);
FL_EXPORT int fl_get_input_xoffset(FL_OBJECT *);

FL_EXPORT int fl_get_input_topline(FL_OBJECT *);
FL_EXPORT int fl_get_input_screenlines(FL_OBJECT *);
FL_EXPORT int fl_get_input_cursorpos(FL_OBJECT *, int *, int *);
FL_EXPORT int fl_get_input_numberoflines(FL_OBJECT *);
FL_EXPORT void fl_get_input_format(FL_OBJECT *, int *, int *);
FL_EXPORT const char *fl_get_input(FL_OBJECT *);

typedef int (*FL_INPUTVALIDATOR) (FL_OBJECT *, const char *, const char *, int);
FL_EXPORT FL_INPUTVALIDATOR fl_set_input_filter(FL_OBJECT *, FL_INPUTVALIDATOR);
#define fl_set_input_shortcut fl_set_object_shortcut

/* edit keys. */
typedef struct
{
    /* basic editing */
    long del_prev_char;		/* delete previous char    */
    long del_next_char;		/* delete next char        */
    long del_prev_word;		/* delete previous word    */
    long del_next_word;		/* delete next word        */

    /* movement */
    long moveto_prev_line;	/* one line  up             */
    long moveto_next_line;	/* one line down            */
    long moveto_prev_char;	/* one char left            */
    long moveto_next_char;	/* one char right           */
    long moveto_prev_word;	/* one word left            */
    long moveto_next_word;	/* one word right           */
    long moveto_prev_page;	/* one page up              */
    long moveto_next_page;	/* one page down            */
    long moveto_bol;		/* move to begining of line */
    long moveto_eol;		/* move to end of line      */
    long moveto_bof;		/* move to begin of file    */
    long moveto_eof;		/* move to end of file      */

    /* misc. stuff */
    long transpose;		/* switch two char positions */
    long paste;			/* paste the edit buffer    */
    long backspace;		/* another  del_prev_char   */
    long del_to_bol;		/* cut to begining of line  */
    long del_to_eol;		/* cut to end of line       */
    long clear_field;		/* delete everything        */
    long del_to_eos;		/* not implemented          */
    long reserverd[4];		/* filler                  */
}
FL_EditKeymap;

FL_EXPORT void fl_set_input_editkeymap(const FL_EditKeymap *);

#endif /* Input_H  } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_MENU_H
#define FL_MENU_H		/* { */


/************   Object Class: Menu         ************/

typedef enum
{
    FL_TOUCH_MENU,
    FL_PUSH_MENU,
    FL_PULLDOWN_MENU
}
FL_MENU_TYPE;

/***** Defaults *****/

#define FL_MENU_BOXTYPE		FL_BORDER_BOX
#define FL_MENU_COL1		FL_COL1
#define FL_MENU_COL2		FL_MCOL
#define FL_MENU_LCOL		FL_LCOL
#define FL_MENU_ALIGN		FL_ALIGN_CENTER

/***** Others   *****/

#define FL_MENU_MAXITEMS	128
#define FL_MENU_MAXSTR		64

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_menu(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				    const char *);

FL_EXPORT FL_OBJECT *fl_add_menu(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				 const char *);

FL_EXPORT void fl_clear_menu(FL_OBJECT *);
FL_EXPORT void fl_set_menu(FL_OBJECT *, const char *);
FL_EXPORT int fl_addto_menu(FL_OBJECT *, const char *);
FL_EXPORT void fl_replace_menu_item(FL_OBJECT *, int, const char *);
FL_EXPORT void fl_delete_menu_item(FL_OBJECT *, int);

FL_EXPORT void fl_set_menu_item_shortcut(FL_OBJECT *, int, const char *);
FL_EXPORT void fl_set_menu_item_mode(FL_OBJECT *, int, unsigned);
FL_EXPORT void fl_show_menu_symbol(FL_OBJECT *, int);
FL_EXPORT void fl_set_menu_popup(FL_OBJECT *, int);
FL_EXPORT int fl_get_menu_popup(FL_OBJECT *);

FL_EXPORT int fl_get_menu(FL_OBJECT *);
FL_EXPORT const char *fl_get_menu_item_text(FL_OBJECT *, int);
FL_EXPORT int fl_get_menu_maxitems(FL_OBJECT *);
FL_EXPORT unsigned fl_get_menu_item_mode(FL_OBJECT *, int);
FL_EXPORT const char *fl_get_menu_text(FL_OBJECT *);
FL_EXPORT int fl_set_menu_entries(FL_OBJECT *, FL_PUP_ENTRY *);

#endif /* MENU } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_MENUBAR_H
#define FL_MENUBAR_H		/* { */


/************   Object Class: MenuBar         ************/

enum
{
    FL_NORMAL_MENUBAR
};

/***** Defaults *****/

#define FL_MENUBAR_BOXTYPE	FL_UP_BOX
#define FL_MENUBAR_COL1		FL_COL1
#define FL_MENUBAR_COL2		FL_MCOL
#define FL_MENUBAR_LCOL		FL_LCOL

/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_menubar(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				 const char *);

FL_EXPORT FL_OBJECT *fl_add_menubar(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
			      const char *);

FL_EXPORT void fl_clear_menubar(FL_OBJECT *);
FL_EXPORT void fl_set_menubar(FL_OBJECT *, const char *);
FL_EXPORT void fl_set_menubar_entries(FL_OBJECT *, const char *, FL_PUP_ENTRY *);

#endif /* MENUBAR } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *  Prototypes for pop-up menus
 */

#ifndef FL_POPUP_H
#define FL_POPUP_H		/* { */


#define FL_MAXPUPI     80	/* max item each pup        */
#define FL_PUP_PADH    4	/* space between each items */

typedef void (*FL_PUP_ENTERCB) (int, void *);	/* call back prototype  */
typedef void (*FL_PUP_LEAVECB) (int, void *);	/* call back prototype  */

FL_EXPORT int fl_setpup_entries(int, FL_PUP_ENTRY *);
FL_EXPORT int fl_newpup(Window);
FL_EXPORT int fl_defpup(Window, const char *,...);
FL_EXPORT int fl_addtopup(int, const char *,...);
FL_EXPORT int fl_setpup_mode(int, int, unsigned);
FL_EXPORT void fl_freepup(int);
FL_EXPORT int fl_dopup(int);

FL_EXPORT Cursor fl_setpup_default_cursor(int);

/* setpup_{colors|fontsize} etc will be removed in next revision
 * use fl_set_default_xxx instead
 */
FL_EXPORT void fl_setpup_color(FL_COLOR, FL_COLOR);
FL_EXPORT void fl_setpup_checkcolor(FL_COLOR);
FL_EXPORT int fl_setpup_fontsize(int);
FL_EXPORT int fl_setpup_fontstyle(int);

#define fl_setpup_default_fontsize    fl_setpup_fontsize
#define fl_setpup_default_fontstyle   fl_setpup_fontstyle
#define fl_setpup_default_color       fl_setpup_color
#define fl_setpup_default_checkcolor  fl_setpup_checkcolor

FL_EXPORT int fl_setpup_default_bw(int);
FL_EXPORT void fl_setpup_shortcut(int, int, const char *);
FL_EXPORT void fl_setpup_position(int, int);
FL_EXPORT void fl_setpup_selection(int, int);
FL_EXPORT void fl_setpup_shadow(int, int);
FL_EXPORT void fl_setpup_softedge(int, int);
FL_EXPORT void fl_setpup_bw(int, int);
FL_EXPORT void fl_setpup_title(int, const char *);
FL_EXPORT FL_PUP_ENTERCB fl_setpup_entercb(int, FL_PUP_ENTERCB, void *);
FL_EXPORT FL_PUP_LEAVECB fl_setpup_leavecb(int, FL_PUP_LEAVECB, void *);
FL_EXPORT void fl_setpup_pad(int, int, int);
FL_EXPORT Cursor fl_setpup_cursor(int, int);
FL_EXPORT int fl_setpup_maxpup(int);
FL_EXPORT unsigned fl_getpup_mode(int, int);
FL_EXPORT const char *fl_getpup_text(int, int);
FL_EXPORT void fl_showpup(int);
FL_EXPORT void fl_hidepup(int);
FL_EXPORT int fl_getpup_items(int);
FL_EXPORT int fl_current_pup(void);

#define fl_setpup_hotkey    fl_setpup_shortcut

FL_EXPORT FL_PUP_CB fl_setpup_itemcb(int, int, FL_PUP_CB);
FL_EXPORT FL_PUP_CB fl_setpup_menucb(int, FL_PUP_CB);
FL_EXPORT void fl_setpup_submenu(int, int, int);

#define fl_setpup    fl_setpup_mode

#endif /* Xpopup } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_POSITIONER_H
#define FL_POSITIONER_H		/* { */


#define FL_NORMAL_POSITIONER	0
#define FL_OVERLAY_POSITIONER	1

/***** Defaults *****/

#define FL_POSITIONER_BOXTYPE	FL_DOWN_BOX
#define FL_POSITIONER_COL1	FL_COL1
#define FL_POSITIONER_COL2	FL_RED
#define FL_POSITIONER_LCOL	FL_LCOL
#define FL_POSITIONER_ALIGN	FL_ALIGN_BOTTOM

/***** Others   *****/


/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_positioner(int, FL_Coord, FL_Coord, FL_Coord,
					  FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_positioner(int, FL_Coord, FL_Coord, FL_Coord,
				       FL_Coord, const char *);

FL_EXPORT void fl_set_positioner_xvalue(FL_OBJECT *, double);
FL_EXPORT double fl_get_positioner_xvalue(FL_OBJECT *);
FL_EXPORT void fl_set_positioner_xbounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_positioner_xbounds(FL_OBJECT *, double *, double *);
FL_EXPORT void fl_set_positioner_yvalue(FL_OBJECT *, double);
FL_EXPORT double fl_get_positioner_yvalue(FL_OBJECT *);
FL_EXPORT void fl_set_positioner_ybounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_positioner_ybounds(FL_OBJECT *, double *, double *);
FL_EXPORT void fl_set_positioner_xstep(FL_OBJECT *, double);
FL_EXPORT void fl_set_positioner_ystep(FL_OBJECT *, double);
FL_EXPORT void fl_set_positioner_return(FL_OBJECT *, int);


#endif /* Positioner.H } */
#ifndef FL_SCROLLBAR_H
#define FL_SCROLLBAR_H

enum
{
    FL_HOR_SCROLLBAR,
    FL_VERT_SCROLLBAR,
    FL_HOR_THIN_SCROLLBAR,
    FL_VERT_THIN_SCROLLBAR,
    FL_HOR_NICE_SCROLLBAR,
    FL_VERT_NICE_SCROLLBAR,
    FL_HOR_PLAIN_SCROLLBAR,
    FL_VERT_PLAIN_SCROLLBAR
};

#define FL_HOR_BASIC_SCROLLBAR      FL_HOR_PLAIN_SCROLLBAR
#define FL_VERT_BASIC_SCROLLBAR     FL_VERT_PLAIN_SCROLLBAR

enum
{
   FL_THIN_SCROLLBAR,
   FL_PLAIN_SCROLLBAR,
   FL_NORMAL_SCROLLBAR,
   FL_NICE_SCROLLBAR
};

FL_EXPORT FL_OBJECT *fl_create_scrollbar(int, FL_Coord, FL_Coord,
				      FL_Coord, FL_Coord, const char *l);
FL_EXPORT FL_OBJECT *fl_add_scrollbar(int, FL_Coord, FL_Coord,
				   FL_Coord, FL_Coord, const char *l);

FL_EXPORT double fl_get_scrollbar_value(FL_OBJECT *);
FL_EXPORT void fl_set_scrollbar_value(FL_OBJECT *, double);
FL_EXPORT void fl_set_scrollbar_size(FL_OBJECT *, double);
FL_EXPORT void fl_set_scrollbar_increment(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_scrollbar_increment(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_set_scrollbar_bounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_scrollbar_bounds(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_set_scrollbar_return(FL_OBJECT *, int);
FL_EXPORT void fl_set_scrollbar_step(FL_OBJECT *, double);

#endif   /* SCROLLBAR */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 * Object Class: Slider
 *
 */
#ifndef FL_SLIDER_H
#define FL_SLIDER_H		/* { */


typedef enum
{
    FL_VERT_SLIDER,
    FL_HOR_SLIDER,
    FL_VERT_FILL_SLIDER,
    FL_HOR_FILL_SLIDER,
    FL_VERT_NICE_SLIDER,
    FL_HOR_NICE_SLIDER,
    FL_HOR_BROWSER_SLIDER,
    FL_VERT_BROWSER_SLIDER,
    FL_HOR_BROWSER_SLIDER2,	/* for internal use only */
    FL_VERT_BROWSER_SLIDER2,	/* for internal use only */
    FL_HOR_THIN_SLIDER,		/* internal use          */
    FL_VERT_THIN_SLIDER,	/* internal use          */
    FL_HOR_BASIC_SLIDER,	/* internal use          */
    FL_VERT_BASIC_SLIDER	/* internal use          */
}
FL_SLIDER_TYPE;


/***** Defaults *****/

#define FL_SLIDER_BW1           FL_BOUND_WIDTH
#define FL_SLIDER_BW2           (FL_abs(FL_BOUND_WIDTH)-1)
#define FL_SLIDER_BOXTYPE	FL_DOWN_BOX
#define FL_SLIDER_COL1		FL_COL1
#define FL_SLIDER_COL2		FL_COL1
#define FL_SLIDER_LCOL		FL_LCOL
#define FL_SLIDER_ALIGN		FL_ALIGN_BOTTOM

/***** Others   *****/

#define FL_SLIDER_FINE		0.05
#define FL_SLIDER_WIDTH		0.10


/***** Routines *****/

FL_EXPORT FL_OBJECT *fl_create_slider(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				      const char *);
FL_EXPORT FL_OBJECT *fl_add_slider(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				   const char *);

FL_EXPORT FL_OBJECT *fl_create_valslider(int, FL_Coord, FL_Coord, FL_Coord,
					 FL_Coord, const char *);
FL_EXPORT FL_OBJECT *fl_add_valslider(int, FL_Coord, FL_Coord, FL_Coord,
				      FL_Coord, const char *);

FL_EXPORT void fl_set_slider_value(FL_OBJECT *, double);
FL_EXPORT double fl_get_slider_value(FL_OBJECT *);
FL_EXPORT void fl_set_slider_bounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_slider_bounds(FL_OBJECT *, double *, double *);

FL_EXPORT void fl_set_slider_return(FL_OBJECT *, int);

FL_EXPORT void fl_set_slider_step(FL_OBJECT *, double);
FL_EXPORT void fl_set_slider_increment(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_slider_increment(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_set_slider_size(FL_OBJECT *, double);
FL_EXPORT void fl_set_slider_precision(FL_OBJECT *, int);
FL_EXPORT void fl_set_slider_filter(FL_OBJECT *,
				 const char *(*)(FL_OBJECT *, double, int));


#endif /* } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_FOLDER_H
#define FL_FOLDER_H		/* { */

enum
{
    FL_TOP_TABFOLDER,		/* tab on top */
    FL_BOTTOM_TABFOLDER,
    FL_LEFT_TABFOLDER,
    FL_RIGHT_TABFOLDER
};

#define FL_NORMAL_TABFOLDER    FL_TOP_TABFOLDER

FL_EXPORT FL_OBJECT *fl_create_tabfolder(int, FL_Coord, FL_Coord, FL_Coord,
					 FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_tabfolder(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				      const char *);

FL_EXPORT FL_OBJECT *fl_addto_tabfolder(FL_OBJECT *, const char *, FL_FORM *);

FL_EXPORT void fl_delete_folder(FL_OBJECT *, FL_FORM *);
FL_EXPORT void fl_delete_folder_bynumber(FL_OBJECT *, int);
FL_EXPORT void fl_delete_folder_byname(FL_OBJECT *, const char *);

FL_EXPORT void fl_set_folder(FL_OBJECT *, FL_FORM *);
FL_EXPORT void fl_set_folder_byname(FL_OBJECT *, const char *);
FL_EXPORT void fl_set_folder_bynumber(FL_OBJECT *, int);

FL_EXPORT FL_FORM *fl_get_folder(FL_OBJECT *);
FL_EXPORT int fl_get_folder_number(FL_OBJECT *);
FL_EXPORT const char *fl_get_folder_name(FL_OBJECT *);

FL_EXPORT FL_FORM *fl_get_active_folder(FL_OBJECT *);
FL_EXPORT int fl_get_active_folder_number(FL_OBJECT *);
FL_EXPORT const char *fl_get_active_folder_name(FL_OBJECT *);

FL_EXPORT void fl_get_folder_area(FL_OBJECT *, FL_Coord *, FL_Coord *,
				  FL_Coord *, FL_Coord *);

FL_EXPORT void fl_replace_folder_bynumber(FL_OBJECT *, int, FL_FORM *);

#endif /* !def FOLDER_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_TEXT_H
#define FL_TEXT_H		/* { */

enum
{
    FL_NORMAL_TEXT
};

#define FL_TEXT_BOXTYPE    FL_FLAT_BOX
#define FL_TEXT_COL1       FL_COL1
#define FL_TEXT_COL2       FL_MCOL
#define FL_TEXT_LCOL       FL_LCOL
#define FL_TEXT_ALIGN      FL_ALIGN_LEFT

FL_EXPORT FL_OBJECT *fl_create_text(int, FL_Coord, FL_Coord, FL_Coord,
                  FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_text(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord, 
                  const char *);

#endif /* !def TEXT_H } */

/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 *  Object Class: Timer
 *
 */
#ifndef FL_TIMER_H
#define FL_TIMER_H		/* { */


typedef enum
{
    FL_NORMAL_TIMER,
    FL_VALUE_TIMER,
    FL_HIDDEN_TIMER
}
FL_TIMER_TYPE;


/***** Defaults *****/

#define FL_TIMER_BOXTYPE	FL_DOWN_BOX
#define FL_TIMER_COL1		FL_COL1
#define FL_TIMER_COL2		FL_RED
#define FL_TIMER_LCOL		FL_LCOL
#define FL_TIMER_ALIGN		FL_ALIGN_CENTER

/***** Others   *****/

#define FL_TIMER_BLINKRATE	0.2

/***** Routines *****/

typedef char *(*FL_TIMER_FILTER) (FL_OBJECT *, double);

FL_EXPORT FL_OBJECT *fl_create_timer(int, FL_Coord, FL_Coord, FL_Coord,
				     FL_Coord, const char *);

FL_EXPORT FL_OBJECT *fl_add_timer(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				  const char *);

FL_EXPORT void fl_set_timer(FL_OBJECT *, double);
FL_EXPORT double fl_get_timer(FL_OBJECT *);
FL_EXPORT void fl_set_timer_countup(FL_OBJECT *, int);
FL_EXPORT FL_TIMER_FILTER fl_set_timer_filter(FL_OBJECT *, FL_TIMER_FILTER);
FL_EXPORT void fl_suspend_timer(FL_OBJECT *);
FL_EXPORT void fl_resume_timer(FL_OBJECT *);


#endif /* TIMER_H } */
/*
 * $Id: forms.h,v 1.1.1.1 2005/01/15 22:02:06 cvs Exp $
 *
 */
#ifndef FL_XYPLOT_H
#define FL_XYPLOT_H		/* { */


/*
 * Class FL_XYPLOT
 */

typedef enum
{
    FL_NORMAL_XYPLOT,		/* solid line                        */
    FL_SQUARE_XYPLOT,		/* with added square                 */
    FL_CIRCLE_XYPLOT,		/* with added circle                 */
    FL_FILL_XYPLOT,		/* fill completely                   */
    FL_POINTS_XYPLOT,		/* only data points                  */
    FL_DASHED_XYPLOT,		/* dashed line                       */
    FL_IMPULSE_XYPLOT,
    FL_ACTIVE_XYPLOT,		/* accepts interactive manipulations */
    FL_EMPTY_XYPLOT,
    FL_DOTTED_XYPLOT,
    FL_DOTDASHED_XYPLOT,
    FL_LONGDASHED_XYPLOT,
    FL_LINEPOINTS_XYPLOT	/* line & points                     */
}
FL_XYPLOT_TYPE;

enum
{
    FL_LINEAR,
    FL_LOG
};

enum
{
    FL_GRID_NONE = 0,
    FL_GRID_MAJOR = 1,
    FL_GRID_MINOR = 2
};

/***** Defaults *****/

#define FL_XYPLOT_BOXTYPE       FL_FLAT_BOX
#define FL_XYPLOT_COL1          FL_COL1
#define FL_XYPLOT_LCOL          FL_LCOL
#define FL_XYPLOT_ALIGN         FL_ALIGN_BOTTOM
#define FL_MAX_XYPLOTOVERLAY    32

/***** Others   *****/


FL_EXPORT FL_OBJECT *fl_create_xyplot(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				      const char *);
FL_EXPORT FL_OBJECT *fl_add_xyplot(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord,
				   const char *);
FL_EXPORT void fl_set_xyplot_data(FL_OBJECT *, float *, float *, int,
				  const char *, const char *, const char *);
FL_EXPORT int fl_set_xyplot_file(FL_OBJECT *, const char *, const char *,
				 const char *, const char *);

#define fl_set_xyplot_datafile fl_set_xyplot_file

FL_EXPORT void fl_add_xyplot_text(FL_OBJECT *, double, double, const char *,
				  int, FL_COLOR);

FL_EXPORT void fl_delete_xyplot_text(FL_OBJECT *, const char *);
FL_EXPORT int fl_set_xyplot_maxoverlays(FL_OBJECT *, int);
FL_EXPORT void fl_add_xyplot_overlay(FL_OBJECT *, int, float *, float *, int, FL_COLOR);
FL_EXPORT int fl_add_xyplot_overlay_file(FL_OBJECT *, int, const char *, FL_COLOR);
FL_EXPORT void fl_set_xyplot_return(FL_OBJECT *, int);
FL_EXPORT void fl_set_xyplot_xtics(FL_OBJECT *, int, int);
FL_EXPORT void fl_set_xyplot_ytics(FL_OBJECT *, int, int);
FL_EXPORT void fl_set_xyplot_xbounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_set_xyplot_ybounds(FL_OBJECT *, double, double);
FL_EXPORT void fl_get_xyplot_xbounds(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_get_xyplot_ybounds(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_get_xyplot(FL_OBJECT *, float *, float *, int *);
FL_EXPORT void fl_get_xyplot_data(FL_OBJECT *, float *, float *, int *);
FL_EXPORT void fl_get_xyplot_overlay_data(FL_OBJECT *, int, float *, float *, int *);
FL_EXPORT void fl_set_xyplot_overlay_type(FL_OBJECT *, int, int);
FL_EXPORT void fl_delete_xyplot_overlay(FL_OBJECT *, int);
FL_EXPORT void fl_set_xyplot_interpolate(FL_OBJECT *, int, int, double);
FL_EXPORT void fl_set_xyplot_inspect(FL_OBJECT *, int);
FL_EXPORT void fl_set_xyplot_symbolsize(FL_OBJECT *, int);
FL_EXPORT void fl_replace_xyplot_point(FL_OBJECT *, int, double, double);
FL_EXPORT void fl_get_xyplot_xmapping(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_get_xyplot_ymapping(FL_OBJECT *, float *, float *);
FL_EXPORT void fl_set_xyplot_keys(FL_OBJECT *, char **, float, float, int);
FL_EXPORT void fl_set_xyplot_key(FL_OBJECT *, int, const char *);
FL_EXPORT void fl_set_xyplot_key_position(FL_OBJECT *, float, float, int);
FL_EXPORT void fl_set_xyplot_key_font(FL_OBJECT *, int, int);
FL_EXPORT int fl_get_xyplot_numdata(FL_OBJECT * ob, int);

/* The following two functions will be removed.
 * Use fl_set_object_l[size|style] for the functionalities
 */
FL_EXPORT void fl_set_xyplot_fontsize(FL_OBJECT *, int);
FL_EXPORT void fl_set_xyplot_fontstyle(FL_OBJECT *, int);

FL_EXPORT void fl_xyplot_s2w(FL_OBJECT *, double, double, float *, float *);
FL_EXPORT void fl_xyplot_w2s(FL_OBJECT *, double, double, float *, float *);
FL_EXPORT void fl_set_xyplot_xscale(FL_OBJECT *, int, double);
FL_EXPORT void fl_set_xyplot_yscale(FL_OBJECT *, int, double);

FL_EXPORT void fl_clear_xyplot(FL_OBJECT *);
FL_EXPORT void fl_set_xyplot_linewidth(FL_OBJECT *, int, int);
FL_EXPORT void fl_set_xyplot_xgrid(FL_OBJECT *, int);
FL_EXPORT void fl_set_xyplot_ygrid(FL_OBJECT *, int);
FL_EXPORT void fl_set_xyplot_alphaxtics(FL_OBJECT *, const char *, const char *);
FL_EXPORT void fl_set_xyplot_alphaytics(FL_OBJECT *, const char *, const char *);
FL_EXPORT void fl_set_xyplot_fixed_xaxis(FL_OBJECT *, const char *, const char *);
FL_EXPORT void fl_set_xyplot_fixed_yaxis(FL_OBJECT *, const char *, const char *);

FL_EXPORT int fl_interpolate(const float *, const float *, int,
			     float *, float *, double, int);

typedef void (*FL_XYPLOT_SYMBOL) (FL_OBJECT *, int, FL_POINT *, int, int, int);
FL_EXPORT FL_XYPLOT_SYMBOL fl_set_xyplot_symbol(FL_OBJECT *, int,
						FL_XYPLOT_SYMBOL);

#endif /* FORM_XYPLOT_H  } */

#if defined(MAKING_FORMS) || defined(MAKING_FD)
#include "flinternal.h"
#endif /* MAKING_FORMS */

#if defined(__cplusplus)
}
#endif

#endif /* FL_FORMS_H */
