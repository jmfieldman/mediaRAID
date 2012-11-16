//
//  MainWindowController.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "MainWindowController.h"

@implementation MainWindowController

- (id) init {
	if ((self = [super init])) {
		[self performSelector:@selector(test) withObject:nil afterDelay:1];
	}
	return self;
}

- (void) test {
	_raidStatusBar.titleText = @"FUCK";
}

@end
