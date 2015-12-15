//
//  MuxOpenFile.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/15/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation

typealias CFileHandle = Int32

class MuxOpenFile {
    
    let fh:         CFileHandle
    let raidpath:   String
    let fullpath:   String
    let muxSource:  MuxSource
    
    init(fh: CFileHandle, raidpath: String, fullpath: String, muxSource: MuxSource) {
        self.fh         = fh
        self.raidpath   = raidpath
        self.fullpath   = fullpath
        self.muxSource  = muxSource
    }
    
    deinit {
        close(fh)
    }
        
}