//
//  RAIDStatusBarView.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "RAIDStatusBarView.h"

@implementation RAIDStatusBarView

- (id)initWithFrame:(NSRect)frame
{
	if ((self = [super initWithFrame:frame])) {
	
		_textField = [[NSTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(5, 5, 100, 15))];
		[_textField setEditable:NO];
		[_textField setSelectable:NO];
		[_textField setBezeled:NO];
		[_textField setBordered:NO];
		[_textField setDrawsBackground:NO];
		[self addSubview:_textField];
		
		
		[self registerForDraggedTypes:@[ NSFilenamesPboardType ]];
		
	}
    
	return self;
}

- (void) setTitleText:(NSString *)titleText {
	_textField.stringValue = titleText;
}

/* Dragging stuff: http://stackoverflow.com/questions/7385352/custom-nsview-drag-destination */

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender {
	return NSDragOperationGeneric;
}

- (void)draggingEnded:(id < NSDraggingInfo >)sender {
	NSLog(@"DRAG: %@", sender);
}

- (BOOL)prepareForDragOperation:(id < NSDraggingInfo >)sender {
	NSLog(@"prepare");
	return YES;
}

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender {
	NSLog(@"perform pasteboard: %@", sender.draggingPasteboard);
	
	NSPasteboard *pboard = [sender draggingPasteboard];
	
    if ([[pboard types] containsObject:NSFilenamesPboardType]) {
		
        NSArray *paths = [pboard propertyListForType:NSFilenamesPboardType];
        for (NSString *path in paths) {
            NSLog(@"PATH: %@", path);
        }
    }
	
	return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
	//[[NSColor colorWithCalibratedWhite:0.75 alpha:1] setFill];
	//NSRectFill([self bounds]);
	
	[[NSColor colorWithCalibratedWhite:0 alpha:0.5] setFill];
	NSRectFill(NSRectFromCGRect(CGRectMake(0, 0, self.bounds.size.width, 1)));
	NSRectFill(NSRectFromCGRect(CGRectMake(0, self.bounds.size.height-1, self.bounds.size.width, 1)));
	
	[[NSColor colorWithCalibratedWhite:1 alpha:0.2] setFill];
	NSRectFill(NSRectFromCGRect(CGRectMake(0, 1, self.bounds.size.width, 1)));
	[[NSColor colorWithCalibratedWhite:1 alpha:0.5] setFill];
	NSRectFill(NSRectFromCGRect(CGRectMake(0, self.bounds.size.height-2, self.bounds.size.width, 1)));
	
	
	
	
	
	
}

@end
