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

		_mountpathField = [[NSTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(5, 0, 300, 30))];
		[_mountpathField setEditable:NO];
		[_mountpathField setSelectable:NO];
		[_mountpathField setBezeled:NO];
		[_mountpathField setBordered:NO];
		[_mountpathField setDrawsBackground:NO];
		[_mountpathField setFont:[NSFont systemFontOfSize:11]];
		NSShadow *shadow = [[NSShadow alloc] init];
		shadow.shadowOffset = NSSizeFromCGSize(CGSizeMake(0, 1));
		shadow.shadowColor = [NSColor colorWithCalibratedWhite:1 alpha:0.75];
		shadow.shadowBlurRadius = 0;
		[_mountpathField setShadow:shadow];
		[self addSubview:_mountpathField];
		
		_mountPointHighlight = [[HighlightView alloc] initWithFrame:NSMakeRect(0, 0, frame.size.width, frame.size.height) allowDrag:YES];
		_mountPointHighlight.dragTarget = TARGET_MOUNT;
		[self addSubview:_mountPointHighlight];
	}
	return self;
}

- (void) setMountpath:(NSString *)mountpath {
	if (!mountpath) mountpath = @"";
	_mountpathField.stringValue = mountpath;
}

- (NSString*) mountpath {
	if (![_mountpathField.stringValue length]) return nil;
	return _mountpathField.stringValue;
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
