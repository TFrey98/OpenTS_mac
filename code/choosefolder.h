/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <string>

// Asks the player to choose a folder with the macOS Open panel, showing the message above the
// folder list. Returns the chosen folder's path, or an empty string when the player cancels.
std::string Choose_Folder(char const * message);

// Shows a modal alert with a single OK button.
void Show_Alert(char const * title, char const * text);
