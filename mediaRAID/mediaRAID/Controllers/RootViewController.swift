//
//  RootViewController.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/13/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation
import Cocoa
import SnapKit

class RootViewController : NSViewController {
 
 
    /**
     Override viewDidLoad
     */
    override func viewDidLoad() {
        
        self.attachToMainWindow()
        
        /* -- Debug -- */
        let v = MuxVolume()
        v.startFUSE()
        
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(3) * Int64(NSEC_PER_SEC)), dispatch_get_main_queue()) {
            v.stopFUSE()
        }
    }
    
    
    /**
     Attach the current controller view to the main window and set min size
     */
    func attachToMainWindow() {
        self.view.snp_makeConstraints { make in
            make.top.equalTo(self.view.superview!).offset(20)
            make.left.equalTo(self.view.superview!)
            make.right.equalTo(self.view.superview!)
            make.bottom.equalTo(self.view.superview!)
            
            make.width.greaterThanOrEqualTo(680).priorityHigh()
            make.height.greaterThanOrEqualTo(560).priorityHigh()
        }
    }
    
}