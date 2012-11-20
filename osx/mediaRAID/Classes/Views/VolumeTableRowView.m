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

		_basepathField = [[NSTextField alloc] initWithFrame:NSMakeRect(5, 44, 630, 20)];
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
		
		_usageField = [[NSTextField alloc] initWithFrame:NSMakeRect(5, 26, 630, 20)];
		[_usageField setEditable:NO];
		[_usageField setSelectable:NO];
		[_usageField setBezeled:NO];
		[_usageField setBordered:NO];
		[_usageField setDrawsBackground:NO];
		[_usageField setFont:[NSFont systemFontOfSize:10]];
		[_usageField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_usageField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		[self addSubview:_usageField];
		
		_replicationStatusField = [[NSTextField alloc] initWithFrame:NSMakeRect(5, 1, 630, 14)];
		[_replicationStatusField setEditable:NO];
		[_replicationStatusField setSelectable:NO];
		[_replicationStatusField setBezeled:NO];
		[_replicationStatusField setBordered:NO];
		[_replicationStatusField setDrawsBackground:NO];
		[_replicationStatusField setFont:[NSFont systemFontOfSize:10]];
		[_replicationStatusField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_replicationStatusField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		[self addSubview:_replicationStatusField];
		
		_freespaceIndicator = [[CleanProgressBar alloc] initWithFrame:NSMakeRect(7, 18, 626, 12)];
		_freespaceIndicator.color = PROGRESS_COLOR_GRAY;
		[self addSubview:_freespaceIndicator];
		
	}
    
	return self;
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
	} else {
		_replicationStatusField.stringValue = @"Replication status: Volume inactive";
	}
}

- (void) setSelected:(BOOL)selected {
	_selected = selected;
	if (selected) {
		[_basepathField          setTextColor:[NSColor colorWithCalibratedWhite:1 alpha:1]];
		[_usageField             setTextColor:[NSColor colorWithCalibratedWhite:1 alpha:1]];
		[_replicationStatusField setTextColor:[NSColor colorWithCalibratedWhite:1 alpha:1]];
	} else {
		[_basepathField          setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_usageField             setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_replicationStatusField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
	}
	
}


@end
