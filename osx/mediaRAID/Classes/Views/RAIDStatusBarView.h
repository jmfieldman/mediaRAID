//
//  RAIDStatusBarView.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface RAIDStatusBarView : NSView {
	NSTextField *_textField;
}

@property (nonatomic, assign) NSString *titleText;

@end
