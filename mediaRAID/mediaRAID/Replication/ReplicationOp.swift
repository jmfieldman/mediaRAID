//
//  ReplicationOp.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/26/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation


class ReplicationOp {
    let raidpath: String
    let isDir:    Bool
    
    init(raidpath: String, isDir: Bool) {
        self.raidpath = raidpath
        self.isDir    = isDir
    }
    
}