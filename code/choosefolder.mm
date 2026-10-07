/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "choosefolder.h"

#import <AppKit/AppKit.h>


// The panels run before SDL has made the game a foreground application, so the game becomes
// one here; a background process's panel would open behind every other window.
static void Become_Foreground_Application(void)
{
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:YES];
}


std::string Choose_Folder(char const * message)
{
	@autoreleasepool {
		Become_Foreground_Application();

		NSOpenPanel * panel = [NSOpenPanel openPanel];
		panel.canChooseFiles = NO;
		panel.canChooseDirectories = YES;
		panel.allowsMultipleSelection = NO;
		panel.prompt = @"Choose";
		panel.message = [NSString stringWithUTF8String:message];

		if ([panel runModal] != NSModalResponseOK || panel.URL == nil) {
			return(std::string());
		}
		return(std::string(panel.URL.fileSystemRepresentation));
	}
}


void Show_Alert(char const * title, char const * text)
{
	@autoreleasepool {
		Become_Foreground_Application();

		NSAlert * alert = [[NSAlert alloc] init];
		alert.messageText = [NSString stringWithUTF8String:title];
		alert.informativeText = [NSString stringWithUTF8String:text];
		[alert addButtonWithTitle:@"OK"];
		[alert runModal];
	}
}
