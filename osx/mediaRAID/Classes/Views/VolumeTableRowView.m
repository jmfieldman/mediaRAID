//
//  VolumeTableRowView.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/17/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "VolumeTableRowView.h"
#import "volumes.h"

@implementation VolumeTableRowView

- (id)initWithFrame:(NSRect)frame {
	if ((self = [super initWithFrame:frame])) {

		_basepathField = [[NSTextField alloc] initWithFrame:NSMakeRect(5, 40, 630, 20)];
		[_basepathField setEditable:NO];
		[_basepathField setSelectable:NO];
		[_basepathField setBezeled:NO];
		[_basepathField setBordered:NO];
		[_basepathField setDrawsBackground:NO];
		[_basepathField setFont:[NSFont systemFontOfSize:14]];
		[_basepathField setTextColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1]];
		[_basepathField setBackgroundColor:[NSColor colorWithCalibratedWhite:0.75 alpha:1]];
		[[_basepathField cell] setLineBreakMode:NSLineBreakByTruncatingHead];
		[self addSubview:_basepathField];
		
		_freespaceIndicator = [[CleanProgressBar alloc] initWithFrame:NSMakeRect(5, 22, 630, 12)];
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
}




@end
