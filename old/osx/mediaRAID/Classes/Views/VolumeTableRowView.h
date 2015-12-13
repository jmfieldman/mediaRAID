//
//  VolumeTableRowView.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/17/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CleanProgressBar.h"
#import "ParentTipButton.h"

@interface VolumeTableRowView : NSView {
	NSTextField          *_basepathField;
	NSTextField          *_usageField;
	NSTextField          *_replicationStatusField;
	NSTextField          *_buttonTooltipField;
	CleanProgressBar     *_freespaceIndicator;
	
	NSImageView          *_activeImageView;
	
	ParentTipButton      *_trashButton;
	ParentTipButton      *_activateButton;
	ParentTipButton      *_finderButton;
}

@property (nonatomic, strong) NSString *basepath;
@property (nonatomic, assign) BOOL      selected;

@end
