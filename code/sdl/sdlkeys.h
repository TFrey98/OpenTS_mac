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

#include <SDL3/SDL_keyboard.h>

#include <string>


// Returns the virtual-key code the active layout gives the key, or 0 for none.
int Virtual_Key_From_SDL(SDL_Scancode scancode, SDL_Keycode keycode, SDL_Keymod modifiers);

// Returns the layout's character or SDL's English name for a virtual-key code, or "" for none.
std::string Virtual_Key_Name(int virtualkey);
