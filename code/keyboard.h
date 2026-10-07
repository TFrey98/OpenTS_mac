/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/keyboard.h                             $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/04/01 9:03p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "_xmouse.h"
#include "win.h"

struct WindowEvent;

enum WWKey_Type {
	WWKEY_SHIFT_BIT	= 0x100,
	WWKEY_CTRL_BIT		= 0x200,
	WWKEY_ALT_BIT		= 0x400,
	WWKEY_RLS_BIT		= 0x800,
	WWKEY_VK_BIT		= 0x1000,
	WWKEY_DBL_BIT		= 0x2000,
	WWKEY_UNK_BIT		= 0x4000,
	WWKEY_BTN_BIT		= 0x8000,
};


class WWKeyboardClass
{
	public:
		/* Define the base constructor and destructors for the class			*/
		WWKeyboardClass(void);

		/* Define the functions which work with the Keyboard Class				*/
		unsigned short Check(void) const;
		unsigned short Get(void);
		bool Put(unsigned short key);
		void Clear(void);
		int To_ASCII(unsigned short num);
		bool Down(unsigned short key);

		/* Define the main hook for the message processing loop.					*/
		bool Handle_Window_Event(WindowEvent const & event);

		/* Define the public access variables which are used with the			*/
		/*   Keyboard Class.																	*/
		int MouseQX;
		int MouseQY;

		Point2D MousePos;

	private:

		/*
		**	This is the circular keyboard holding buffer. It holds the VK key and
		**	the current shift state at the time the key was added to the queue.
		*/
		unsigned short Buffer[256];		// buffer which holds actual keypresses

		// The character each queued entry typed, at the entry's position in Buffer, or 0.
		char32_t Text[256];

		// The queued key press that the next typed character belongs to, or -1.
		int TextSlot;

		// Set while a held key repeats; the characters it repeats are not queued.
		bool DropText;

		// The entry the last Get returned, and the character it typed.
		unsigned short FetchedKey;
		char32_t FetchedText;

		unsigned short Buff_Get(void);
		unsigned short Fetch_Element(void);
		unsigned short Peek_Element(void) const;
		bool Put_Element(unsigned short val);
		bool Is_Buffer_Full(void) const;
		bool Is_Buffer_Empty(void) const;
		static bool Is_Mouse_Key(unsigned short key);
		void Fill_Buffer_From_System(void);
		bool Put_Key_Message(unsigned short vk_key, bool release = false, int modifiers = 0);
		bool Put_Mouse_Message(unsigned short vk_key, int x, int y, bool release = false);
		bool Put_Text(char32_t code);
		int Available_Buffer_Room(void) const;
		int Noop(void) const; /// Empty routine added in a TS patch.

		/*
		**	These are the tracking pointers to maintain the
		**	circular keyboard list.
		*/
		int Head;
		int Tail;
};


#include "vkey.h"

#define VK_UPLEFT	 VK_HOME
#define VK_UPRIGHT	 VK_PRIOR
#define VK_DOWNLEFT	 VK_END
#define VK_DOWNRIGHT VK_NEXT
#define VK_ALT		 VK_MENU

enum KeyASCIIType {
	//
	// Define all the KA types as variations of the VK types.  This is
	// so the KA functions will work properly under windows 95.
	//
	KA_NONE = 0,
	KA_MORE = 1,
	KA_SETBKGDCOL = 2,
	KA_SETFORECOL = 6,
	KA_FORMFEED = 12,
	KA_SPCTAB = 20,
	KA_SETX = 25,
	KA_SETY = 26,

	KA_SPACE = 32,	/*   */
	KA_EXCLAMATION, /* ! */
	KA_DQUOTE,		/* " */
	KA_POUND,		/* # */
	KA_DOLLAR,		/* $ */
	KA_PERCENT,		/* % */
	KA_AMPER,		/* & */
	KA_SQUOTE,		/* ' */
	KA_LPAREN,		/* ( */
	KA_RPAREN,		/* ) */
	KA_ASTERISK,	/* * */
	KA_PLUS,		/* + */
	KA_COMMA,		/* , */
	KA_MINUS,		/* - */
	KA_PERIOD,		/* . */
	KA_SLASH,		/* / */

	KA_0,
	KA_1,
	KA_2,
	KA_3,
	KA_4,
	KA_5,
	KA_6,
	KA_7,
	KA_8,
	KA_9,
	KA_COLON,		 /* : */
	KA_SEMICOLON,	 /* ; */
	KA_LESS_THAN,	 /* < */
	KA_EQUAL,		 /* = */
	KA_GREATER_THAN, /* > */
	KA_QUESTION,	 /* ? */

	KA_AT, /* @ */
	KA_A,  /* A */
	KA_B,  /* B */
	KA_C,  /* C */
	KA_D,  /* D */
	KA_E,  /* E */
	KA_F,  /* F */
	KA_G,  /* G */
	KA_H,  /* H */
	KA_I,  /* I */
	KA_J,  /* J */
	KA_K,  /* K */
	KA_L,  /* L */
	KA_M,  /* M */
	KA_N,  /* N */
	KA_O,  /* O */

	KA_P,		  /* P */
	KA_Q,		  /* Q */
	KA_R,		  /* R */
	KA_S,		  /* S */
	KA_T,		  /* T */
	KA_U,		  /* U */
	KA_V,		  /* V */
	KA_W,		  /* W */
	KA_X,		  /* X */
	KA_Y,		  /* Y */
	KA_Z,		  /* Z */
	KA_LBRACKET,  /* [ */
	KA_BACKSLASH, /* \ */
	KA_RBRACKET,  /* ] */
	KA_CARROT,	  /* ^ */
	KA_UNDERLINE, /* _ */

	KA_GRAVE, /* ` */
	KA_a,	  /* a */
	KA_b,	  /* b */
	KA_c,	  /* c */
	KA_d,	  /* d */
	KA_e,	  /* e */
	KA_f,	  /* f */
	KA_g,	  /* g */
	KA_h,	  /* h */
	KA_i,	  /* i */
	KA_j,	  /* j */
	KA_k,	  /* k */
	KA_l,	  /* l */
	KA_m,	  /* m */
	KA_n,	  /* n */
	KA_o,	  /* o */

	KA_p,	   /* p */
	KA_q,	   /* q */
	KA_r,	   /* r */
	KA_s,	   /* s */
	KA_t,	   /* t */
	KA_u,	   /* u */
	KA_v,	   /* v */
	KA_w,	   /* w */
	KA_x,	   /* x */
	KA_y,	   /* y */
	KA_z,	   /* z */
	KA_LBRACE, /* { */
	KA_BAR,	   /* | */
	KA_RBRACE, /* ] */
	KA_TILDA,  /* ~ */

	KA_ESC = VK_ESCAPE,
	KA_EXTEND = VK_ESCAPE,
	KA_RETURN = VK_RETURN,
	KA_BACKSPACE = VK_BACK,
	KA_TAB = VK_TAB,
	KA_DELETE = VK_DELETE, /* <DELETE> */
	KA_INSERT = VK_INSERT, /* <INSERT> */
	KA_PGDN = VK_NEXT,	   /* <PAGE DOWN> */
	KA_DOWNRIGHT = VK_NEXT,
	KA_DOWN = VK_DOWN, /* <DOWN ARROW> */
	KA_END = VK_END,   /* <END> */
	KA_DOWNLEFT = VK_END,
	KA_RIGHT = VK_RIGHT,	/* <RIGHT ARROW> */
	KA_KEYPAD5 = VK_SELECT, /* NUMERIC KEY PAD <5> */
	KA_LEFT = VK_LEFT,		/* <LEFT ARROW> */
	KA_PGUP = VK_PRIOR,		/* <PAGE UP> */
	KA_UPRIGHT = VK_PRIOR,
	KA_UP = VK_UP,	   /* <UP ARROW> */
	KA_HOME = VK_HOME, /* <HOME> */
	KA_UPLEFT = VK_HOME,
	KA_F12 = VK_F12,
	KA_F11 = VK_F11,
	KA_F10 = VK_F10,
	KA_F9 = VK_F9,
	KA_F8 = VK_F8,
	KA_F7 = VK_F7,
	KA_F6 = VK_F6,
	KA_F5 = VK_F5,
	KA_F4 = VK_F4,
	KA_F3 = VK_F3,
	KA_F2 = VK_F2,
	KA_F1 = VK_F1,
	KA_LMOUSE = VK_LBUTTON,
	KA_RMOUSE = VK_RBUTTON,

	KA_SHIFT_BIT = WWKEY_SHIFT_BIT,
	KA_CTRL_BIT = WWKEY_CTRL_BIT,
	KA_ALT_BIT = WWKEY_ALT_BIT,
	KA_RLSE_BIT = WWKEY_RLS_BIT,
};


enum KeyNumType {
	KN_NONE = 0,

	KN_0 = VK_0,
	KN_1 = VK_1,
	KN_2 = VK_2,
	KN_3 = VK_3,
	KN_4 = VK_4,
	KN_5 = VK_5,
	KN_6 = VK_6,
	KN_7 = VK_7,
	KN_8 = VK_8,
	KN_9 = VK_9,
	KN_A = VK_A,
	KN_B = VK_B,
	KN_BACKSLASH = VK_NONE_DC,
	KN_BACKSPACE = VK_BACK,
	KN_C = VK_C,
	KN_CAPSLOCK = VK_CAPITAL,
	KN_CENTER = VK_CLEAR,
	KN_COMMA = VK_NONE_BC,
	KN_D = VK_D,
	KN_DELETE = VK_DELETE,
	KN_DOWN = VK_DOWN,
	KN_DOWNLEFT = VK_END,
	KN_DOWNRIGHT = VK_NEXT,
	KN_E = VK_E,
	KN_END = VK_END,
	KN_EQUAL = VK_NONE_BB,
	KN_ESC = VK_ESCAPE,
	KN_E_DELETE = VK_DELETE,
	KN_E_DOWN = VK_NUMPAD2,
	KN_E_END = VK_NUMPAD1,
	KN_E_HOME = VK_NUMPAD7,
	KN_E_INSERT = VK_INSERT,
	KN_E_LEFT = VK_NUMPAD4,
	KN_E_PGDN = VK_NUMPAD3,
	KN_E_PGUP = VK_NUMPAD9,
	KN_E_RIGHT = VK_NUMPAD6,
	KN_E_UP = VK_NUMPAD8,
	KN_F = VK_F,
	KN_F1 = VK_F1,
	KN_F10 = VK_F10,
	KN_F11 = VK_F11,
	KN_F12 = VK_F12,
	KN_F2 = VK_F2,
	KN_F3 = VK_F3,
	KN_F4 = VK_F4,
	KN_F5 = VK_F5,
	KN_F6 = VK_F6,
	KN_F7 = VK_F7,
	KN_F8 = VK_F8,
	KN_F9 = VK_F9,
	KN_G = VK_G,
	KN_GRAVE = VK_NONE_C0,
	KN_H = VK_H,
	KN_HOME = VK_HOME,
	KN_I = VK_I,
	KN_INSERT = VK_INSERT,
	KN_J = VK_J,
	KN_K = VK_K,
	KN_KEYPAD_ASTERISK = VK_MULTIPLY,
	KN_KEYPAD_MINUS = VK_SUBTRACT,
	KN_KEYPAD_PLUS = VK_ADD,
	KN_KEYPAD_RETURN = VK_RETURN,
	KN_KEYPAD_SLASH = VK_DIVIDE,
	KN_L = VK_L,
	KN_LALT = VK_MENU,
	KN_LBRACKET = VK_NONE_DB,
	KN_LCTRL = VK_CONTROL,
	KN_LEFT = VK_LEFT,
	KN_LMOUSE = VK_LBUTTON,
	KN_LSHIFT = VK_SHIFT,
	KN_M = VK_M,
	KN_MINUS = VK_NONE_BD,
	KN_N = VK_N,
	KN_NUMLOCK = VK_NUMLOCK,
	KN_O = VK_O,
	KN_P = VK_P,
	KN_PAUSE = VK_PAUSE,
	KN_PERIOD = VK_NONE_BE,
	KN_PGDN = VK_NEXT,
	KN_PGUP = VK_PRIOR,
	KN_PRNTSCRN = VK_PRINT,
	KN_Q = VK_Q,
	KN_R = VK_R,
	KN_RALT = VK_MENU,
	KN_RBRACKET = VK_NONE_DD,
	KN_RCTRL = VK_CONTROL,
	KN_RETURN = VK_RETURN,
	KN_RIGHT = VK_RIGHT,
	KN_RMOUSE = VK_RBUTTON,
	KN_RSHIFT = VK_SHIFT,
	KN_S = VK_S,
	KN_SCROLLLOCK = VK_SCROLL,
	KN_SEMICOLON = VK_NONE_BA,
	KN_SLASH = VK_NONE_BF,
	KN_SPACE = VK_SPACE,
	KN_SQUOTE = VK_NONE_DE,
	KN_T = VK_T,
	KN_TAB = VK_TAB,
	KN_U = VK_U,
	KN_UP = VK_UP,
	KN_UPLEFT = VK_HOME,
	KN_UPRIGHT = VK_PRIOR,
	KN_V = VK_V,
	KN_W = VK_W,
	KN_X = VK_X,
	KN_Y = VK_Y,
	KN_Z = VK_Z,

	// A queue entry for typed text that no key press carries; no key has this code.
	KN_TEXT = 0xFF,

	KN_SHIFT_BIT = WWKEY_SHIFT_BIT,
	KN_CTRL_BIT = WWKEY_CTRL_BIT,
	KN_ALT_BIT = WWKEY_ALT_BIT,
	KN_RLSE_BIT = WWKEY_RLS_BIT,
	KN_UNK = WWKEY_UNK_BIT,
	KN_BUTTON = WWKEY_BTN_BIT,
};


/*
**	Interface class to the keyboard. This insulates the game from library vagaries. Most
**	notable being the return values are declared as "int" in the library whereas C&C
**	expects it to be of KeyNumType.
*/

//lint -esym(1725,KeyboardClass::MouseQX,KeyboardClass::MouseQY)
struct KeyboardClass : public WWKeyboardClass
{
	/*
	**	This flag is used to indicate whether the WW library has taken over
	**	the keyboard or not. If not, then the normal console input
	**	takes precedence.
	*/
	unsigned IsLibrary;

	KeyboardClass(void) : IsLibrary(true) {}
	KeyNumType Get(void) {return((KeyNumType)WWKeyboardClass::Get());};
	KeyNumType Check(void) {return((KeyNumType)WWKeyboardClass::Check());};
	KeyASCIIType To_ASCII(KeyNumType key) {return((KeyASCIIType)WWKeyboardClass::To_ASCII(key));};
	void Clear(void) {WWKeyboardClass::Clear();};
	int Down(KeyNumType key) {return(WWKeyboardClass::Down(key));};

	int Mouse_X(void) {return(Get_Mouse_X());};
	int Mouse_Y(void) {return(Get_Mouse_Y());};
};
