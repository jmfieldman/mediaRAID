//
//  AppDelegate.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	[self addStatusBarItem];
	
	[self.window setReleasedWhenClosed:NO];
	[self.window makeKeyAndOrderFront:self];
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag {
	[self.window makeKeyAndOrderFront:self];
	return YES;
}

- (void) addStatusBarItem {
	NSStatusBar *bar = [NSStatusBar systemStatusBar];
	
	_statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
	[_statusItem setImage:[NSImage imageNamed:@"statusbar_off"]];
	[_statusItem setHighlightMode:YES];
	
	[_statusItem setTarget:self];
	[_statusItem setAction:@selector(statusBarIconClicked)];
}

- (void) statusBarIconClicked {
	[[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
	[self.window makeKeyAndOrderFront:self];
	
}

/* Hijack the command-Q behavior to just close the window */
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
	[self.window close];
	return NSTerminateCancel;
}

@end
