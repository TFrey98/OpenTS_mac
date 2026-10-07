/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "win.h"

class Surface;
struct SurfaceFont;

// Returns a cached TrueType font, or nullptr when no file for the face can be loaded. Sizes
// follow Win32 CreateFont: a negative height is the em size in pixels and a positive one the
// cell height, and a nonzero width is the average character width in pixels.
SurfaceFont * Surface_Font(char const * face, int height, int width, bool bold);

// Measures UTF-8 text: the width is the sum of the advances and the height is the cell height.
void Surface_Text_Extent(SurfaceFont * font, char const * text, int length, int & width, int & height);

// Draws UTF-8 text with the top left of its cell at x and y, antialiased against a 16-bit
// 5-6-5 surface's existing pixels and clipped to the surface. Returns the advance width.
int Surface_Draw_Text(Surface & surface, SurfaceFont * font, int x, int y, char const * text, int length, COLORREF color);
