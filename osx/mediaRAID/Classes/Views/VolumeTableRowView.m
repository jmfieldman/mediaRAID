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

		
		_basepathField = [[NSTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(5, 0, 300, 30))];
		[_basepathField setEditable:NO];
		[_basepathField setSelectable:NO];
		[_basepathField setBezeled:NO];
		[_basepathField setBordered:NO];
		[_basepathField setDrawsBackground:NO];
		[_basepathField setFont:[NSFont systemFontOfSize:11]];
		NSShadow *shadow = [[NSShadow alloc] init];
		shadow.shadowOffset = NSSizeFromCGSize(CGSizeMake(0, 1));
		shadow.shadowColor = [NSColor colorWithCalibratedWhite:1 alpha:0.75];
		shadow.shadowBlurRadius = 0;
		[_basepathField setShadow:shadow];
		[self addSubview:_basepathField];
		
	}
    
	return self;
}


- (void) setBasepath:(NSString *)basepath {
	_basepath = basepath;
	_basepathField.stringValue = basepath;
}




@end
