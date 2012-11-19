//
//  VolumeTableRowView.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/17/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CleanProgressBar.h"

@interface VolumeTableRowView : NSView {
	NSTextField          *_basepathField;
	CleanProgressBar     *_freespaceIndicator;
}

@property (nonatomic, strong) NSString *basepath;

@end
