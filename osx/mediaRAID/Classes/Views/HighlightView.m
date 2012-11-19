//
//  HighlightView.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/19/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "HighlightView.h"

@implementation HighlightView

- (id)initWithFrame:(NSRect)frame allowDrag:(BOOL)allowDrag {
	if ((self = [super initWithFrame:frame])) {
		_allowDragging = allowDrag;
		if (allowDrag) {
			_isDragging = NO;
			[self registerForDraggedTypes:@[ NSFilenamesPboardType ]];
		} else {
			_isDragging = YES;
		}
	}
	return self;
}

- (void)drawRect:(NSRect)dirtyRect {
	[[NSColor colorWithCalibratedRed:0 green:0.5 blue:1 alpha:((_isDragging || !_allowDragging) ? 0.2 : 0)] setFill];
	NSRectFill([self bounds]);
}

- (NSView*) hitTest:(NSPoint)aPoint {
	return nil;
}


/* --------------------------------------------------------------------------------------------------------------- */
/* Dragging stuff: http://stackoverflow.com/questions/7385352/custom-nsview-drag-destination */

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender {
	_isDragging = YES;
	[self setNeedsDisplay:YES];
	NSLog(@"drag in");
	return NSDragOperationCopy;
}

- (void)draggingExited:(id < NSDraggingInfo >)sender {
	_isDragging = NO;
	[self setNeedsDisplay:YES];
	NSLog(@"drag out");
}

- (void)draggingEnded:(id < NSDraggingInfo >)sender {
	_isDragging = NO;
	[self setNeedsDisplay:YES];
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
			
			if (_dragTarget == TARGET_VOLUME) {
				NSDictionary *dic = [NSDictionary dictionaryWithObject:path forKey:@"basepath"];
				[[NSNotificationCenter defaultCenter] postNotificationName:kRequestNewVolumeNotification object:self userInfo:dic];
			}
			
        }
    }
	
	return YES;
}

/* --------------------------------------------------------------------------------------------------------------- */

@end
