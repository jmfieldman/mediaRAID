//
//  ParentTipButton.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/20/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "ParentTipButton.h"

@implementation ParentTipButton

- (id)initWithFrame:(NSRect)frame {
	if ((self = [super initWithFrame:frame])) {
	}
	return self;
}


- (void)mouseEntered:(NSEvent *)event {
	if (_tooltip) {
		[self.superview performSelector:@selector(displayButtonTip:) withObject:_tooltip];
	}
}

- (void)mouseExited:(NSEvent *)event {
	if (_tooltip) {
		[self.superview performSelector:@selector(displayButtonTip:) withObject:nil];
	}
}

- (void) updateTrackingAreas {
	[super updateTrackingAreas];
	
	NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0, 0, self.frame.size.width, self.frame.size.height) options:NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways owner:self userInfo:nil];
	[self addTrackingArea:area];
}

/*
- (void)_updateMouseTracking {
    [super _updateMouseTracking];
    if ([self controlView] != nil && [[self controlView] respondsToSelector:@selector(_setMouseTrackingForCell:)]) {
        [[self controlView] performSelector:@selector(_setMouseTrackingForCell:) withObject:self];
    }
}
 */

@end
