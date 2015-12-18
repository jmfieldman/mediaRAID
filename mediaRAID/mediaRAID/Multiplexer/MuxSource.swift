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
class MuxSource : Equatable {
    
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
    
    func fileExistsAtRAIDPath(path: String) -> (exists: Bool, isDir: Bool) {
        var isDir: ObjCBool = false
        let exists = NSFileManager.defaultManager().fileExistsAtPath(self.raidpath + path, isDirectory: &isDir)
        return (exists: exists, isDir: Bool(isDir))
    }
    
    func fileExistsAtFullPath(path: String) -> (exists: Bool, isDir: Bool) {
        var isDir: ObjCBool = false
        let exists = NSFileManager.defaultManager().fileExistsAtPath(path, isDirectory: &isDir)
        return (exists: exists, isDir: Bool(isDir))
    }
    
    func ensureParentDirectoryForFullPath(path: String) -> Bool {
        let newpathParent = (path as NSString).stringByDeletingLastPathComponent
        
        do {
            try NSFileManager.defaultManager().createDirectoryAtPath(newpathParent, withIntermediateDirectories: true, attributes: nil)
        } catch {
            return false
        }
        return true
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
    
    func os_rename(oldpath: String, newpath: String) -> Int32 {
        let fulloldpath = self.raidpath + oldpath
        let fullnewpath = self.raidpath + newpath
        
        if (!NSFileManager.defaultManager().fileExistsAtPath(fulloldpath)) {
            errno = ENOENT
            return -ENOENT
        }
        
        let newpathParent = (fullnewpath as NSString).stringByDeletingLastPathComponent
        let (parentNeedsCreation, isDir) = self.fileExistsAtFullPath(newpathParent)
        
        if (!isDir) {
            errno = ENOTDIR
            return -ENOTDIR
        }
        
        if (parentNeedsCreation) {
            if (!ensureParentDirectoryForFullPath(fullnewpath)) {
                errno = EIO
                return -EIO
            }
        }
        
        let res = rename(fulloldpath, fullnewpath)
        if (res != 0 && parentNeedsCreation) {
            try! NSFileManager.defaultManager().removeItemAtPath(newpathParent)
        }
        
        return res
    }
    
    func os_unlink(path: String) -> Int32 {
        return unlink(self.raidpath + path)
    }
    
    func os_access(path: String, amode: Int32) -> Int32 {
        return access(self.raidpath + path, amode)
    }
    
    func os_rmdir(path: String) -> Int32 {
        return rmdir(self.raidpath + path)
    }
    
    func os_mkdir(path: String, mode: mode_t) -> Int32 {
        
        let fullpath = self.raidpath + path
        
        if (!ensureParentDirectoryForFullPath(fullpath)) {
            errno = EIO
            return -EIO
        }
        
        return mkdir(fullpath, mode)
    }
    
    func os_chmod(path: String, mode: mode_t) -> Int32 {
        return chmod(self.raidpath + path, mode)
    }
    
    func os_chown(path: String, uid: uid_t, gid: gid_t) -> Int32 {
        return chown(self.raidpath + path, uid, gid)
    }
    
    func os_truncate(path: String, length: off_t) -> Int32 {
        return truncate(self.raidpath + path, length)
    }
    
}

func ==(lhs: MuxSource, rhs: MuxSource) -> Bool {
    return lhs.basepath == rhs.basepath
}
