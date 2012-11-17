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
	//[self.window close];
	[self.window makeKeyAndOrderFront:self];
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag {
	[self.window makeKeyAndOrderFront:self];
	return YES;
}

- (void) addStatusBarItem {
	NSStatusBar *bar = [NSStatusBar systemStatusBar];
	
	_statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
    [_statusItem setTitle: NSLocalizedString(@"mediaRAID",@"")];
    [_statusItem setHighlightMode:YES];
	
	[_statusItem setTarget:self];
	[_statusItem setAction:@selector(fuck)];
}

- (void) fuck {
	//activateIgnoringOtherApps
	[[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
	[self.window makeKeyAndOrderFront:self];
	
}

@end
