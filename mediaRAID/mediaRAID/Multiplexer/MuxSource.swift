//
//  MuxSource.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/13/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation

/**
The MuxSource class identifies a single source that feeds into a MuxVolume
*/
class MuxSource {
    
    /** The basepath is the path to the pre-"mediaRAID" directory.
        This path must contain the mediaRAID and .mediaRAID directories
     */
    let basepath: String
    
    /** The raidpath is the full basepath + "/mediaRAID" directory */
    let raidpath: String
    
    /** The workpath is the full basepath + "/.mediaRAID" directory */
    let workpath: String
    
    
    init(basepath: String) {
        self.basepath = basepath
        self.raidpath = basepath + "/mediaRAID"
        self.workpath = basepath + "/.mediaRAID"
    }
  
    func capacityInfo() -> (total: UInt64, free: UInt64)? {
        var sbuf = statvfs()
        let err = statvfs(raidpath, &sbuf)
        
        if (err != 0) {
            return nil
        }
        
        return (total: UInt64(sbuf.f_frsize) * UInt64(sbuf.f_blocks),
                 free: UInt64(sbuf.f_frsize) * UInt64(sbuf.f_bavail))
    }
    
}


// MARK: - Operating System Shims

extension MuxSource {
    
    func os_stat(path: String, inout stbuf: stat) -> Int32 {
        let fullpath = self.raidpath + path
        let err      = stat(fullpath, &stbuf)
        return err
    }
    
    func os_statvfs(path: String, statbuf: UnsafeMutablePointer<statvfs>) -> Int32 {
        let fullpath = self.raidpath + path
        return statvfs(fullpath, statbuf)
    }
}

func ==(lhs: MuxSource, rhs: MuxSource) -> Bool {
    return lhs.basepath == rhs.basepath
}
