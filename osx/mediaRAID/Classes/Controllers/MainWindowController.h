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

@interface MainWindowController : NSObject

@property (strong) IBOutlet RAIDStatusBarView    *raidStatusBar;
@property (strong) IBOutlet RAIDStatusBarView    *volumeStatusBar;
@property (strong) IBOutlet MountPointInfoView   *mountPointInfoView;

@property (strong) IBOutlet NSButton             *testButton;


@end
