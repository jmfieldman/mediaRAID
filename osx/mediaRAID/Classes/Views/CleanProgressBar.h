//
//  CleanProgressBar.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/19/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define PROGRESS_COLOR_BLUE [NSColor colorWithCalibratedRed:0.35 green:0.67 blue:0.98 alpha:0.50]
#define PROGRESS_COLOR_GRAY [NSColor colorWithCalibratedRed:0.00 green:0.00 blue:0.00 alpha:0.16]

@interface CleanProgressBar : NSView

@property (nonatomic, assign) double   progress;
@property (nonatomic, strong) NSColor *color;

@end
