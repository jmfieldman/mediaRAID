//
//  MainWindowController.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "MainWindowController.h"
#import "AppDelegate.h"
#import "VolumeTableRowView.h"

@implementation MainWindowController

- (id) init {
	if ((self = [super init])) {

	}
	return self;
}

- (void) awakeFromNib {
	
	_raidStatusBar.titleText   = @"Mount Point Information";
	_raidStatusBar.yOffset     = 60;
	
	_volumeStatusBar.titleText = @"Volume Information";
	_volumeStatusBar.yOffset   = 60 + 72 + 22;
	
	[_testButton setTarget:self];
	[_testButton setAction:@selector(performClick:)];
	
	_volumeTableView.delegate   = self;
	_volumeTableView.dataSource = self;
	[_volumeTableView reloadData];
}

- (void) performClick:(id)sender {
	NSLog(@"clicked");
	
	AppDelegate *d = [NSApplication sharedApplication].delegate;
	NSRect foo = NSRectFromCGRect(CGRectMake(d.window.frame.origin.x, d.window.frame.origin.y, 500, 500));
	//foo = NSRectFromCGRect(CGRectMake(100, 100, 500, 500));
	
	NSRect oldFrame = d.window.frame;
	oldFrame.origin.y += 1;
	
	[d.window setFrame:foo display:NO];
	
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
	frameSize.width = 480;
	return frameSize;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
	return 2;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	NSLog(@"VCR %@ %ld", tableColumn, row);
	
	VolumeTableRowView *rowView = [[VolumeTableRowView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(1,1,1,1))];
	return rowView;
	
	return nil;
}

/*
- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row {
	
	NSLog(@"row query: %ld", row);
	
	VolumeTableRowView *rowView = [[VolumeTableRowView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(1,1,1,1))];
	
	NSTableRowView *rv = [[NSTableRowView alloc] initWithFrame:NSRectFromCGRect(CGRectZero)];
	return nil;
	
	//return nil;
	return rowView;
}
 */

@end
