//
//  FormatHelper.m
//  mediaRAID
//
//  Created by Jason Fieldman on 11/19/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import "FormatHelper.h"

@implementation FormatHelper

+ (NSString*) prettyFilesize:(int64_t)bytes {
	
	int64_t lim = 1024 * 1024;
	
	if (bytes < lim) {
		return [NSString stringWithFormat:@"%.2lf KB", (double)bytes / (lim >> 10)];
	}
	
	lim *= 1024;
	
	if (bytes < lim) {
		return [NSString stringWithFormat:@"%.2lf MB", (double)bytes / (lim >> 10)];
	}
	
	lim *= 1024;
	
	if (bytes < lim) {
		return [NSString stringWithFormat:@"%.2lf GB", (double)bytes / (lim >> 10)];
	}
	
	lim *= 1024;
	
	return [NSString stringWithFormat:@"%.2lf TB", (double)bytes / (lim >> 10)];
}

@end
