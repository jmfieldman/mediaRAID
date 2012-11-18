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
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(notificationRequestNewVolume:) name:kRequestNewVolumeNotification object:nil];
		
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
	[_volumeTableView registerForDraggedTypes:@[ NSFilenamesPboardType ]];
	[_volumeTableView reloadData];
	
	
	/* Restore existing volumes to the list */
	[self restoreVolumesFromDefaults];
	
	
}


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
	return volume_count(0) + volume_count(1);
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

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes {
	NSLog(@"SHITSHITHSITHISTHSITH");
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation {
	NSLog(@"SHITSHITHSITHISTHSITH 2");
	return NSDragOperationCopy;
}

- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation {
	NSLog(@"SHITSHITHSITHISTHSITH 3");
	
	NSPasteboard *pboard = [info draggingPasteboard];
	
    if ([[pboard types] containsObject:NSFilenamesPboardType]) {
		
        NSArray *paths = [pboard propertyListForType:NSFilenamesPboardType];
        for (NSString *path in paths) {
            NSLog(@"PATH: %@", path);
			
			NSDictionary *dic = [NSDictionary dictionaryWithObject:path forKey:@"basepath"];
			[[NSNotificationCenter defaultCenter] postNotificationName:kRequestNewVolumeNotification object:self userInfo:dic];
        }
    }
	
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
	
	return rowView;
}


@end
