//
//  FormatHelper.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/19/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface FormatHelper : NSObject

+ (NSString*) prettyFilesize:(int64_t)bytes;

@end
