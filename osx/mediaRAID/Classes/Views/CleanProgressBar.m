//
//  CleanProgressBar.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/19/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "CleanProgressBar.h"

@implementation CleanProgressBar

- (id)initWithFrame:(NSRect)frame {
	if ((self = [super initWithFrame:frame])) {
		_progress = 0;
		_color = PROGRESS_COLOR_BLUE;
	}
    
	return self;
}

- (void) setProgress:(double)progress {
	_progress = progress;
	[self setNeedsDisplay:YES];
}

- (void) setColor:(NSColor *)color {
	_color = color;
	[self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect {
	
	NSRect b = [self bounds];
	
	[[NSColor colorWithCalibratedWhite:0.8 alpha:1] setFill];
	NSRectFill(NSMakeRect(0, 0, b.size.width, b.size.height/2));
	
	[[NSColor colorWithCalibratedWhite:0.9 alpha:1] setFill];
	NSRectFill(NSMakeRect(0, 0, b.size.width, b.size.height/3));
	
	[[NSColor colorWithCalibratedWhite:0.65 alpha:1] setFill];
	NSFrameRect(b);

	[_color setFill];
	NSRect r = NSMakeRect(0, 0, b.size.width * _progress, b.size.height);
	NSRectFillUsingOperation(r, NSCompositeSourceOver);
}

@end
