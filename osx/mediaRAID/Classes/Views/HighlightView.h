//
//  HighlightView.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/19/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define TARGET_VOLUME 0
#define TARGET_MOUNT  1

@interface HighlightView : NSView {
	BOOL _isDragging;
	BOOL _allowDragging;
}

@property (assign) int dragTarget;

- (id)initWithFrame:(NSRect)frame allowDrag:(BOOL)allowDrag;

@end
