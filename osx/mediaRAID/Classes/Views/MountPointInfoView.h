//
//  MountPointInfoView.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "HighlightView.h"

@interface MountPointInfoView : NSView {
	NSTextField *_mountpathField;
}

@property (strong) HighlightView   *mountPointHighlight;
@property (strong) NSString        *mountpath;

@end
