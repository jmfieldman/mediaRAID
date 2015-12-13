//
//  MainWindowController.h
//  mediaRAID
//
//  Created by Jason Fieldman on 11/16/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RAIDStatusBarView.h"
#import "MountPointInfoView.h"
#import "VolumeTableView.h"
#import "HighlightView.h"

#define WINDOW_WIDTH 640

@interface MainWindowController : NSObject <NSTableViewDelegate, NSTableViewDataSource>

@property (strong) IBOutlet RAIDStatusBarView    *raidStatusBar;
@property (strong) IBOutlet RAIDStatusBarView    *volumeStatusBar;
@property (strong) IBOutlet MountPointInfoView   *mountPointInfoView;
@property (strong) IBOutlet VolumeTableView      *volumeTableView;

@property (strong) HighlightView *volumeTableHighlight;


@property (strong) IBOutlet NSButton             *testButton;

- (CGFloat) windowHeightForVolumeRows:(int)numrows;

@end
