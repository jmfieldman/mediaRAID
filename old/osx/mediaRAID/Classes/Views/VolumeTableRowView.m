//
//  VolumeTableRowView.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/17/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "VolumeTableRowView.h"
#import "volumes.h"
#import "FormatHelper.h"

@implementation VolumeTableRowView

- (id)initWithFrame:(NSRect)frame {
	if ((self = [super initWithFrame:frame])) {

		_activeImageView = [[NSImageView alloc] initWithFrame:NSMakeRect(5, 24, 16, 16)];
		_activeImageView.image = [NSImage imageNamed:@"dot_green"];
		[self addSubview:_activeImageView];
		
		_basepathField = [[NSTextField alloc] initWithFrame:NSMakeRect(25, 44, 610, 20)];
		[_basepathField setEditable:NO];
		[_basepathField setSelectable:NO];
		[_basepathField setBezeled:NO];
		[_basepathField setBordered:NO];
		[_basepathField setDrawsBackground:NO];
		[_basepathField setFont:[NSFont boldSystemFontOfSize:12]];
		[_basepathField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_basepathField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		[[_basepathField cell] setLineBreakMode:NSLineBreakByTruncatingHead];
		[self addSubview:_basepathField];
		
		_usageField = [[NSTextField alloc] initWithFrame:NSMakeRect(25, 26, 610, 20)];
		[_usageField setEditable:NO];
		[_usageField setSelectable:NO];
		[_usageField setBezeled:NO];
		[_usageField setBordered:NO];
		[_usageField setDrawsBackground:NO];
		[_usageField setFont:[NSFont systemFontOfSize:10]];
		[_usageField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_usageField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		[self addSubview:_usageField];
		
		_replicationStatusField = [[NSTextField alloc] initWithFrame:NSMakeRect(25, 1, 500, 14)];
		[_replicationStatusField setEditable:NO];
		[_replicationStatusField setSelectable:NO];
		[_replicationStatusField setBezeled:NO];
		[_replicationStatusField setBordered:NO];
		[_replicationStatusField setDrawsBackground:NO];
		[_replicationStatusField setFont:[NSFont systemFontOfSize:10]];
		[_replicationStatusField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_replicationStatusField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		[self addSubview:_replicationStatusField];
		
		_buttonTooltipField = [[NSTextField alloc] initWithFrame:NSMakeRect(25, 1, 610, 14)];
		[_buttonTooltipField setEditable:NO];
		[_buttonTooltipField setSelectable:NO];
		[_buttonTooltipField setBezeled:NO];
		[_buttonTooltipField setBordered:NO];
		[_buttonTooltipField setDrawsBackground:NO];
		[_buttonTooltipField setFont:[NSFont systemFontOfSize:10]];
		[_buttonTooltipField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_buttonTooltipField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		_buttonTooltipField.alignment = 1;
		[self addSubview:_buttonTooltipField];
		
		_freespaceIndicator = [[CleanProgressBar alloc] initWithFrame:NSMakeRect(27, 18, 540, 12)];
		_freespaceIndicator.color = PROGRESS_COLOR_GRAY;
		[self addSubview:_freespaceIndicator];
		
		_trashButton = [[ParentTipButton alloc] initWithFrame:NSMakeRect(615, 18, 16, 16)];
		[_trashButton setButtonType:NSMomentaryChangeButton];
		[_trashButton setImage:[NSImage imageNamed:@"trash_mini"]];
		[_trashButton setBordered:NO];
		_trashButton.tooltip = @"Remove Volume";
		[_trashButton setTarget:self];
		[_trashButton setAction:@selector(pressedTrash:)];
		[self addSubview:_trashButton];
		
		_activateButton = [[ParentTipButton alloc] initWithFrame:NSMakeRect(595, 18, 16, 16)];
		[_activateButton setButtonType:NSMomentaryChangeButton];
		[_activateButton setImage:[NSImage imageNamed:@"activate_mini"]];
		[_activateButton setBordered:NO];
		_activateButton.tooltip = @"Activate Volume";
		[_activateButton setTarget:self];
		[_activateButton setAction:@selector(pressedActivateToggle:)];
		[self addSubview:_activateButton];
		
		_finderButton = [[ParentTipButton alloc] initWithFrame:NSMakeRect(575, 18, 16, 16)];
		[_finderButton setButtonType:NSMomentaryChangeButton];
		[_finderButton setImage:[NSImage imageNamed:@"find_mini"]];
		[_finderButton setBordered:NO];
		_finderButton.tooltip = @"Show in Finder";
		[_finderButton setAction:@selector(showInFinder:)];
		[_finderButton setTarget:self];
		[self addSubview:_finderButton];
		
	}
    
	return self;
}

- (void) pressedTrash:(id)sender {
	NSDictionary *dic = [NSDictionary dictionaryWithObject:_basepath forKey:@"basepath"];
	[[NSNotificationCenter defaultCenter] postNotificationName:kRequestRemoveVolumeNotification object:self userInfo:dic];
}

- (void) pressedActivateToggle:(id)sender {
	RaidVolume_t *vol = volume_with_basepath([_basepath UTF8String]);
	if (!vol) return;
	
	NSDictionary *dic = [NSDictionary dictionaryWithObject:_basepath forKey:@"basepath"];
	[[NSNotificationCenter defaultCenter] postNotificationName:(vol->active ? kRequestDeactivateVolumeNotification : kRequestActivateVolumeNotification) object:self userInfo:dic];
}

- (void) showInFinder:(id)sender {
	BOOL isdir;
	if (![[NSFileManager defaultManager] fileExistsAtPath:_basepath isDirectory:&isdir] || !isdir) {
		NSAlert *alert = [NSAlert alertWithMessageText:@"Bad Volume Path" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"The volume path could not be found on this computer.  It may have been deleted or disconnected?"];
		[alert runModal];
	}
	
	[[NSWorkspace sharedWorkspace] selectFile:_basepath inFileViewerRootedAtPath:@""];
}

- (void) displayButtonTip:(NSString*)tip {
	if (!tip) tip = @"";
	_buttonTooltipField.stringValue = tip;
}

- (void) setBasepath:(NSString *)basepath {
	_basepath = basepath;
	_basepathField.stringValue = basepath;
	
	RaidVolume_t *info = volume_with_basepath([basepath UTF8String]);
	if (!info) return;
	volume_update_all_byte_counters();
	
	_freespaceIndicator.progress = (double)info->capacity_used / (double)info->capacity_total;
	_freespaceIndicator.color = (info->active) ? PROGRESS_COLOR_BLUE : PROGRESS_COLOR_GRAY;
	
	_usageField.stringValue = [NSString stringWithFormat:@"Used: %@ of %@ (%.2lf%%); %@ free",
							   [FormatHelper prettyFilesize:info->capacity_used],
							   [FormatHelper prettyFilesize:info->capacity_total],
							   100 * (double)info->capacity_used / (double)info->capacity_total,
							   [FormatHelper prettyFilesize:info->capacity_free]
							   ];
	
	if (info->active) {
		_replicationStatusField.stringValue = [NSString stringWithFormat:@"Replication status: %s", info->replication_status_string];
		_activateButton.tooltip = @"Deactivate Volume";
		[_activateButton setImage:[NSImage imageNamed:@"deactivate_mini"]];
		_activeImageView.image = [NSImage imageNamed:@"dot_green"];
	} else {
		_replicationStatusField.stringValue = @"Replication status: Volume inactive";
		_activateButton.tooltip = @"Activate Volume";
		[_activateButton setImage:[NSImage imageNamed:@"activate_mini"]];
		_activeImageView.image = [NSImage imageNamed:@"dot_red"];
	}
}

- (void) setSelected:(BOOL)selected {
	_selected = selected;
	if (selected) {
		[_basepathField          setTextColor:[NSColor colorWithCalibratedWhite:1 alpha:1]];
		[_usageField             setTextColor:[NSColor colorWithCalibratedWhite:1 alpha:1]];
		[_replicationStatusField setTextColor:[NSColor colorWithCalibratedWhite:1 alpha:1]];
		[_buttonTooltipField     setTextColor:[NSColor colorWithCalibratedWhite:1 alpha:1]];
	} else {
		[_basepathField          setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_usageField             setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_replicationStatusField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_buttonTooltipField     setTextColor:[NSColor colorWithCalibratedWhite:0.50 alpha:1]];
	}
	
}


@end
