//
//  RAIDStatusBarView.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "RAIDStatusBarView.h"

@implementation RAIDStatusBarView

- (void) awakeFromNib {
	self.autoresizingMask = 0;
	
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize {
	NSRect f = self.frame;
	f.size.width = oldBoundsSize.width;
	f.origin.y = oldBoundsSize.height - _yOffset;
	self.frame = f;
	
}

- (id)initWithFrame:(NSRect)frame {
	if ((self = [super initWithFrame:frame])) {
	
		
		_textField = [[NSTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(5, -4, 300, self.frame.size.height))];
		[_textField setEditable:NO];
		[_textField setSelectable:NO];
		[_textField setBezeled:NO];
		[_textField setBordered:NO];
		[_textField setDrawsBackground:NO];
		[_textField setFont:[NSFont systemFontOfSize:11]];
		NSShadow *shadow = [[NSShadow alloc] init];
		shadow.shadowOffset = NSSizeFromCGSize(CGSizeMake(0, 1));
		shadow.shadowColor = [NSColor colorWithCalibratedWhite:1 alpha:0.75];
		shadow.shadowBlurRadius = 0;
		[_textField setShadow:shadow];
		[self addSubview:_textField];
		
	}
    
	return self;
}

- (void) setTitleText:(NSString *)titleText {
	_textField.stringValue = titleText;
}


- (void)drawRect:(NSRect)dirtyRect
{
	//[[NSColor colorWithCalibratedWhite:1 alpha:0.2] setFill];
	[[NSColor colorWithCalibratedWhite:0.9 alpha:1] setFill];
	NSRectFill([self bounds]);
	
	[[NSColor colorWithCalibratedWhite:0 alpha:0.3] setFill];
	NSRectFill(NSRectFromCGRect(CGRectMake(0, 0, self.bounds.size.width, 1)));
	NSRectFill(NSRectFromCGRect(CGRectMake(0, self.bounds.size.height-1, self.bounds.size.width, 1)));
	
	[[NSColor colorWithCalibratedWhite:1 alpha:0.2] setFill];
	NSRectFill(NSRectFromCGRect(CGRectMake(0, 1, self.bounds.size.width, 1)));
	[[NSColor colorWithCalibratedWhite:1 alpha:0.5] setFill];
	NSRectFill(NSRectFromCGRect(CGRectMake(0, self.bounds.size.height-2, self.bounds.size.width, 1)));
	
}

@end
