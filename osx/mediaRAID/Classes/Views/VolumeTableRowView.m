//
//  VolumeTableRowView.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/17/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "VolumeTableRowView.h"

@implementation VolumeTableRowView

- (id)initWithFrame:(NSRect)frame {
	if ((self = [super initWithFrame:frame])) {

		
		NSTextField *_textField = [[NSTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(5, 0, 300, 30))];
		[_textField setEditable:NO];
		[_textField setSelectable:NO];
		[_textField setBezeled:NO];
		[_textField setBordered:NO];
		[_textField setDrawsBackground:NO];
		[_textField setFont:[NSFont systemFontOfSize:11]];
		_textField.stringValue = @"FUCK";
		NSShadow *shadow = [[NSShadow alloc] init];
		shadow.shadowOffset = NSSizeFromCGSize(CGSizeMake(0, 1));
		shadow.shadowColor = [NSColor colorWithCalibratedWhite:1 alpha:0.75];
		shadow.shadowBlurRadius = 0;
		[_textField setShadow:shadow];
		[self addSubview:_textField];
		
		
	}
    
	return self;
}


@end
