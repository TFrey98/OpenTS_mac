/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ownrdraw.h"

#include "_surface.h"
#include "_xmouse.h"
#include "arraylist.h"
#include "dsurface.h"
#include "globals.h"
#include "keyboard.h"
#include "misc.h"
#include "sdl/sdlwindow.h"
#include "surface.h"
#include "surftext.h"

#include <cstring>


using namespace OwnerDraw;


static int _mouse_counter;

unsigned short ODRComponentMask;
unsigned short ODGComponentMask;
unsigned short ODBComponentMask;


/// <summary>
/// Builds the color component masks used for blending.
/// The masks depend on how the display surface packs its pixels, so this cannot run
/// before the video mode is set.
/// </summary>
void ODInitMasks(void)
{
	ODRComponentMask = 255;
	ODRComponentMask = ODRComponentMask >> DSurface::Get_Red_Left();
	ODRComponentMask <<= DSurface::Get_Red_Right();

	ODGComponentMask = 255;
	ODGComponentMask = ODGComponentMask >> DSurface::Get_Green_Left();
	ODGComponentMask <<= DSurface::Get_Green_Right();

	ODBComponentMask = 255;
	ODBComponentMask = ODBComponentMask >> DSurface::Get_Blue_Left();
	ODBComponentMask <<= DSurface::Get_Blue_Right();
}


/// <summary>
/// Builds the blend masks once; call it after the video mode is set.
/// </summary>
void OwnerDraw::Prepare_Resources(void)
{
	static bool _inited = false;
	if (!_inited) {
		ODInitMasks();
		_inited = true;
	}
}


/// <summary>
/// Names a hotkey for the player, as its modifiers and then its key, for example
/// "Ctrl+Shift+A".
/// </summary>
std::string Build_Hotkey_String(KeyNumType key)
{
	unsigned const code = (unsigned)key;
	std::string name;

	if ((code & WWKEY_ALT_BIT) != 0) {
		name += Main_Window_Key_Name(VK_MENU) + "+";
	}
	if ((code & WWKEY_CTRL_BIT) != 0) {
		name += Main_Window_Key_Name(VK_CONTROL) + "+";
	}
	if ((code & WWKEY_SHIFT_BIT) != 0) {
		name += Main_Window_Key_Name(VK_SHIFT) + "+";
	}

	name += Main_Window_Key_Name(code & 0xFF);
	return(name);
}


#define EZ_ATTR_BOLD		  1


/// <summary>
/// Fetches a font of the typeface and size requested. Fonts are cached, so a repeated request
/// returns the same font. The height is in tenths of a point, which the game's identity
/// mapping makes pixels. Only the bold attribute and a width of zero, which lets the typeface
/// choose its aspect, are supported; no caller asks for more.
/// </summary>
/// <returns>Returns with the font, or NULL if no file for the typeface could be loaded.</returns>
/// <remarks>The returned font stays owned by the cache. Do not delete it.</remarks>
SurfaceFont * WS_Get_Font(const char * face_name, int, int decipt_height, int attributes)
{
	return(Surface_Font(face_name, -decipt_height, 0, (attributes & EZ_ATTR_BOLD) != 0));
}


/// <summary>
/// Draws a line of text into the rectangle on a surface, aligned as asked.
/// A full-screen game that does not hold the focus draws nothing and returns zero.
/// </summary>
/// <param name="len">The number of bytes of the UTF-8 text to draw.</param>
/// <param name="surface">The surface to draw upon, or NULL to draw on the alternate
/// surface.</param>
/// <returns>Returns with the pixel width of the text.</returns>
int OD_Draw_Text(COLORREF color, SurfaceFont * font, Rect const & rect, const char * text, int len, int x_alignment, int y_alignment, Surface * surface)
{
	if (!GameInFocus && !WindowedMode) {
		return(0);
	}

	Surface * destsurf = (surface != NULL) ? surface : AlternateSurface;
	if (destsurf == NULL || font == NULL) {
		return(0);
	}

	int textwidth = 0;
	int textheight = 0;
	Surface_Text_Extent(font, text, len, textwidth, textheight);

	int x_offset = rect.X;
	int y_offset = rect.Y;

	if (x_alignment == OD_TEXT_ALIGN_MIN) {
		x_offset += (rect.Width - textwidth + 1) / 2;
	} else if (x_alignment == OD_TEXT_ALIGN_CENTER) {
		x_offset += (textwidth + 1) / -2;
	} else if (x_alignment == OD_TEXT_ALIGN_MAX) {
		x_offset += -1 - textwidth;
	}

	if (y_alignment == OD_TEXT_ALIGN_MIN) {
		y_offset += (rect.Height - textheight + 1) / 2;
	} else if (y_alignment == OD_TEXT_ALIGN_CENTER) {
		y_offset += (textheight + 1) / -2;
	} else if (y_alignment == OD_TEXT_ALIGN_MAX) {
		y_offset += -1 - textheight;
	}

	Surface_Draw_Text(*destsurf, font, x_offset, y_offset, text, len, color);
	return(textwidth);
}


/// <summary>
/// Takes the mouse away from the game so that a dialog may use it.
/// The game cursor gives up its capture, leaving Windows free to drive the dialog and its
/// controls.
/// </summary>
/// <returns>Returns with the number of captures now outstanding.</returns>
/// <remarks>Each call must be matched by a call to Release_Mouse.</remarks>
int OwnerDraw::Capture_Mouse(void)
{
	if (MouseCursor != NULL) {
		if (MouseCursor->Is_Captured() == true) {
			MouseCursor->Release_Mouse();
		}
	}
	_mouse_counter++;
	return(_mouse_counter);
}


/// <summary>
/// Gives the mouse back to the game.
/// This routine undoes one Capture_Mouse. Only when the last dialog has finished with the
/// mouse does the game cursor take it back.
/// </summary>
/// <returns>Returns with the number of captures still outstanding.</returns>
int OwnerDraw::Release_Mouse(void)
{
	if (_mouse_counter > 0) {
		_mouse_counter--;
	}
	if (_mouse_counter == 0) {
		if (MouseCursor != NULL) {
			if (!MouseCursor->Is_Captured()) {
				MouseCursor->Capture_Mouse();
			}
		}
	}
	return(_mouse_counter);
}
