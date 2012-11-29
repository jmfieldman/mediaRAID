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
#import "volumes.h"

@implementation MainWindowController

- (id) init {
	if ((self = [super init])) {
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notificationRequestNewVolume:)         name:kRequestNewVolumeNotification         object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notificationRequestNewMount:)          name:kRequestNewMountNotification          object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notificationRequestRemoveVolume:)      name:kRequestRemoveVolumeNotification      object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notificationRequestActivateVolume:)    name:kRequestActivateVolumeNotification    object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notificationRequestDeactivateVolume:)  name:kRequestDeactivateVolumeNotification  object:nil];
		
	}
	return self;
}

- (void) awakeFromNib {
		
	/* Setup sizing */
	_raidStatusBar.titleText   = @"Mount Point Information";
	_raidStatusBar.yOffset     = 60;
	
	_volumeStatusBar.titleText = @"Volume Information";
	_volumeStatusBar.yOffset   = 60 + 72 + 22;
		
	/* Table delegate stuff */
	_volumeTableView.delegate   = self;
	_volumeTableView.dataSource = self;
	[_volumeTableView reloadData];
	
	/* Set the default mount path */
	_mountPointInfoView.mountpath = [self savedMountPoint];
	
	/* Restore existing volumes to the list */
	[self restoreVolumesFromDefaults];
	
	/* Attach table highlight view */
	_volumeTableHighlight = [[HighlightView alloc] initWithFrame:_volumeTableView.frame allowDrag:YES];
	_volumeTableHighlight.dragTarget = TARGET_VOLUME;
	[[_volumeTableView superview] addSubview:_volumeTableHighlight];	
}

#pragma mark Volume stuff

- (void) restoreVolumesFromDefaults {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	NSArray *volumes = [defaults objectForKey:@"volumes"];
	if (!volumes) return;
	
	for (NSDictionary *volume in volumes) {
		NSString *basepath = [volume objectForKey:@"basepath"];
		BOOL active = [[volume objectForKey:@"active"] boolValue];

		const char *charpath = [basepath UTF8String];
		RaidVolume_t *vol = create_volume(charpath, charpath, NULL, NULL, NULL);
		volume_set_active(vol, active);
	}
	
	[_volumeTableView reloadData];
}

- (void) saveVolumesToDefaults {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	RaidVolume_t *actives[256];
	RaidVolume_t *inactives[256];
	int active_count = 255;
	int inactive_count = 255;
	
	memset(actives,   0, sizeof(actives));
	memset(inactives, 0, sizeof(inactives));
	
	volume_get_all(actives, &active_count, inactives, &inactive_count);
	
	NSMutableArray *volumes = [NSMutableArray array];
	
	for (int i = 0; i < active_count; i++) {
		NSMutableDictionary *volume = [NSMutableDictionary dictionary];
		[volume setObject:[NSNumber numberWithBool:YES] forKey:@"active"];
		[volume setObject:[NSString stringWithUTF8String:actives[i]->basepath] forKey:@"basepath"];
		[volumes addObject:volume];
	}
	
	for (int i = 0; i < inactive_count; i++) {
		NSMutableDictionary *volume = [NSMutableDictionary dictionary];
		[volume setObject:[NSNumber numberWithBool:NO] forKey:@"active"];
		[volume setObject:[NSString stringWithUTF8String:inactives[i]->basepath] forKey:@"basepath"];
		[volumes addObject:volume];
	}
	
	[defaults setObject:volumes forKey:@"volumes"];
	[defaults synchronize];
}

- (void) notificationRequestNewVolume:(NSNotification*)notification {
	NSString *basepath = [notification.userInfo objectForKey:@"basepath"];
	const char *path = [basepath UTF8String];
	NSLog(@"Adding path: %@", basepath);
	RaidVolume_t *vol = create_volume(path, path, NULL, NULL, NULL);
	volume_set_active(vol, 1);
	printf("vol basepath: %s\n", vol->basepath);
	[_volumeTableView reloadData];
	NSLog(@"vol count: %d %d", volume_count(0), volume_count(1));
	
	[self saveVolumesToDefaults];
}

- (void) notificationRequestRemoveVolume:(NSNotification*)notification {

	NSString *basepath = [notification.userInfo objectForKey:@"basepath"];
	const char *path = [basepath UTF8String];
	
	RaidVolume_t *vol = volume_with_basepath(path);
	if (!vol) return;
	
	volume_set_active(vol, NO);
	volume_remove(vol);
	
	[_volumeTableView reloadData];
	[self saveVolumesToDefaults];
	
}

- (void) notificationRequestActivateVolume:(NSNotification*)notification {
	NSString *basepath = [notification.userInfo objectForKey:@"basepath"];
	const char *path = [basepath UTF8String];
	
	RaidVolume_t *vol = volume_with_basepath(path);
	if (!vol) return;
	
	volume_set_active(vol, YES);
	
	[_volumeTableView reloadData];
	[self saveVolumesToDefaults];
}

- (void) notificationRequestDeactivateVolume:(NSNotification*)notification {
	NSString *basepath = [notification.userInfo objectForKey:@"basepath"];
	const char *path = [basepath UTF8String];
	
	RaidVolume_t *vol = volume_with_basepath(path);
	if (!vol) return;
	
	volume_set_active(vol, NO);
	
	[_volumeTableView reloadData];
	[self saveVolumesToDefaults];
}


#pragma mark Mount Point stuff

- (NSString*) savedMountPoint {
	NSUserDefaults *def = [NSUserDefaults standardUserDefaults];
	return [def stringForKey:@"mountpath"];
}

- (void) setSavedMountPoint:(NSString*)mountpath {
	NSUserDefaults *def = [NSUserDefaults standardUserDefaults];
	[def setObject:mountpath forKey:@"mountpath"];
}

- (void) notificationRequestNewMount:(NSNotification*)notification {
	NSString *mountpath = [notification.userInfo objectForKey:@"mountpath"];
	//const char *path = [mountpath UTF8String];
	NSLog(@"New mount: %@", mountpath);

	[self setSavedMountPoint:mountpath];
	_mountPointInfoView.mountpath = mountpath;
}


#pragma mark Resizing stuff

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
	int rows = volume_count(0) + volume_count(1);
	if (rows < 1) rows = 1;
	CGFloat newHeight = [self windowHeightForVolumeRows:rows];
	
	frameSize.width = WINDOW_WIDTH;
	frameSize.height = newHeight;
	return frameSize;
}

- (void)windowDidResize:(NSNotification *)notification {
	_volumeTableHighlight.frame = _volumeTableView.frame;

}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)newFrame {
	NSLog(@"new frame");
	
	NSRect current = window.frame;
	CGFloat ytop = current.origin.y + current.size.height;
	
	CGFloat newHeight = [self windowHeightForVolumeRows:volume_count(0) + volume_count(1)];
	if (newHeight > newFrame.size.height) newHeight = newFrame.size.height;
	NSRect resFrame = NSMakeRect(current.origin.x, ytop - newHeight, current.size.width, newHeight );
	
	return resFrame;
}

- (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame {
	NSLog(@"should zoom");
	return YES;
}

- (CGFloat) windowHeightForVolumeRows:(int)numrows {
	return (numrows * 65) + (22 * 3) + 72 + 66;
}

#pragma mark NSTableViewDelegate methods

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
	return volume_count(0) + volume_count(1);
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex {
	
	return NO;
	
	NSInteger lastSelected = [aTableView selectedRow];
	if (lastSelected >= 0) {
		VolumeTableRowView *row = [aTableView viewAtColumn:0 row:lastSelected makeIfNecessary:NO];
		row.selected = NO;
	}
	
	VolumeTableRowView *row = [aTableView viewAtColumn:0 row:rowIndex makeIfNecessary:NO];
	row.selected = YES;
	
	return YES;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	
	VolumeTableRowView *rowView = nil;//[tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
	if (!rowView) {
		rowView = [[VolumeTableRowView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(1,1,1,1))];
	}
	
	RaidVolume_t *actives[256];
	RaidVolume_t *inactives[256];
	int active_count = 255;
	int inactive_count = 255;
	
	memset(actives,   0, sizeof(actives));
	memset(inactives, 0, sizeof(inactives));
	
	volume_get_all(actives, &active_count, inactives, &inactive_count);
	
	RaidVolume_t *target = NULL;
	
	if (active_count > row) {
		target = actives[row];
	} else {
		row -= active_count;
		target = inactives[row];
	}
	
	if (!target) {
		//EXLog(VOLUME, ERR, @"Invalid volume row count");
		return nil;
	}
	
	printf("target bp: %s\n", target->basepath);
	NSString *bp = [NSString stringWithUTF8String:target->basepath];
	rowView.basepath = bp;
	NSLog(@"Set bp: %@", bp);
	
	rowView.selected = NO;
	
	return rowView;
}


@end
