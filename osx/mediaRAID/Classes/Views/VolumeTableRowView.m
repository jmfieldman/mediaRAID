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

		_basepathField = [[NSTextField alloc] initWithFrame:NSMakeRect(5, 45, 630, 20)];
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
		
		_usageField = [[NSTextField alloc] initWithFrame:NSMakeRect(5, 28, 630, 20)];
		[_usageField setEditable:NO];
		[_usageField setSelectable:NO];
		[_usageField setBezeled:NO];
		[_usageField setBordered:NO];
		[_usageField setDrawsBackground:NO];
		[_usageField setFont:[NSFont systemFontOfSize:10]];
		[_usageField setTextColor:[NSColor colorWithCalibratedWhite:0.0 alpha:1]];
		[_usageField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		[self addSubview:_usageField];
		
		_freespaceIndicator = [[CleanProgressBar alloc] initWithFrame:NSMakeRect(5, 19, 630, 12)];
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
	
	_usageField.stringValue = [NSString stringWithFormat:@"Used: %@ of %@ (%.2lf%%); %@ free",
							   [FormatHelper prettyFilesize:info->capacity_used],
							   [FormatHelper prettyFilesize:info->capacity_total],
							   100 * (double)info->capacity_used / (double)info->capacity_total,
							   [FormatHelper prettyFilesize:info->capacity_free]
							   ];
}




@end
