//
//  MountPointInfoView.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "MountPointInfoView.h"

@implementation MountPointInfoView

- (id)initWithFrame:(NSRect)frame {
    if ((self = [super initWithFrame:frame])) {

		_mountPointHighlight = [[HighlightView alloc] initWithFrame:NSMakeRect(0, 0, frame.size.width, frame.size.height) allowDrag:YES];
		_mountPointHighlight.dragTarget = TARGET_MOUNT;
		[self addSubview:_mountPointHighlight];
	}
	return self;
}

- (void) awakeFromNib {
	self.autoresizingMask = 0;
}

- (void)drawRect:(NSRect)dirtyRect {
	[[NSColor colorWithCalibratedWhite:1 alpha:1] setFill];
	NSRectFill([self bounds]);
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize {
	NSRect f = self.frame;
	f.size.width = oldBoundsSize.width;
	f.origin.y = oldBoundsSize.height - (60 + 72);
	self.frame = f;
}


@end
