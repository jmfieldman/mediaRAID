//
//  MuxVolume.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/10/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation

enum MuxStartError {
    case CouldNotMount
    case CouldNotStartSession
    case CouldNotInstallSigHandlers
}


/**
The MuxVolume class combines several MuxSources into a single virtual volume
*/
class MuxVolume {
    
    /* -- Support Mux volume hash -- */
    private static var _volumeHashQueue  = dispatch_queue_create("mediaRAID.MuxVolume.volumeHashQueue",  DISPATCH_QUEUE_CONCURRENT)
    private static var _volumeIndexQueue = dispatch_queue_create("mediaRAID.MuxVolume.volumeIndexQueue", DISPATCH_QUEUE_CONCURRENT)
    
    private static var _volumeHash: [Int64 : MuxVolume] = [:]
    private static var _nextVolumeIndex: Int64 = 1111
    
    /* FUSE properties */
    private var fuseChannel: COpaquePointer = nil
    private var fuseSession: COpaquePointer = nil
    
    /* Used in the global FUSE initialization */
    var __unsafeVolumeIndexPtr  = UnsafeMutablePointer<Int64>.alloc(1)
    var __wasFUSEd              = false
    
    /** The unique hash index of this volume */
    let volumeIndex             = MuxVolume.nextVolumeIndex()
    
    /** The last FUSE error code */
    var lastFUSEerror           = 0
    
    /** The sources for this mux volume */
    var sources: [MuxSource]    = []
    
    init() {
        MuxVolume.addVolumeToHash(self)
        __unsafeVolumeIndexPtr.initialize(volumeIndex)
    }
    
    deinit {
        __unsafeVolumeIndexPtr.destroy()
    }
 
    
    /* -- Start FUSE -- */
    
    func startFUSE() -> MuxStartError? {
        
        let fuseQueue = dispatch_queue_create("mediaRAID.MuxVolume.FUSEQueue \(volumeIndex)", nil)
        
        let arguments: [String] = ["", "-o", "default_permissions", "-o", "allow_other"]
        var cargs = arguments.map { strdup($0) }
        var fargs: fuse_args = fuse_args(argc: Int32(cargs.count), argv: &cargs, allocated: 0)
        var fops = multiplex_operations()
        
        fuseChannel = fuse_mount("/tmp/fusefoo", &fargs)
        if (fuseChannel == nil) {
            return .CouldNotMount
        }
        
        fuseSession = fuse_new(fuseChannel, &fargs, &fops, sizeof(fuse_operations), nil)
        if (fuseSession == nil) {
            fuse_unmount("/tmp/fusefoo", fuseChannel)
            return .CouldNotStartSession
        }
        
        if fuse_set_signal_handlers(fuseSession) != 0 {
            fuse_session_destroy(fuseSession)
            fuse_unmount("/tmp/fusefoo", fuseChannel)
            return .CouldNotInstallSigHandlers
        }
        
        dispatch_async(fuseQueue) {
            
            OSSpinLockLock(&__currentMuxVolumeIndexLock)
            __currentMuxVolumeIndex = unsafeBitCast(self.__unsafeVolumeIndexPtr, UnsafeMutablePointer<Void>.self)
            
            
            let err = fuse_loop(self.fuseSession)
            
            if (!self.__wasFUSEd) {
                OSSpinLockUnlock(&__currentMuxVolumeIndexLock)
            }
            
            
            print("err: \(err)")
            
            fuse_remove_signal_handlers(self.fuseSession)
            fuse_session_destroy(self.fuseSession)
            
            fuse_unmount("/tmp/fusefoo", self.fuseChannel)
            
            print("done!")
        }
        
        return nil
    }
 
    
    func stopFUSE() {
        //fuse_unmount("/tmp/fusefoo", fuseChannel)
    }
}





// MARK: - FUSE Operations

extension MuxVolume {
    
    /* ---------------------------------------------------------- */
    /* -------------------- FUSE OPERATIONS --------------------- */
    /* ---------------------------------------------------------- */
    
    func os_initialize() {
        __wasFUSEd = true
    }
    
    func os_getattr(path: String, stbuf: UnsafeMutablePointer<stat>) -> Int32 {
        return 0
    }
    
    func os_fgetattr(path: String, stbuf: UnsafeMutablePointer<stat>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
        return 0
    }
    
    func os_statfs(path: String, statbuf: UnsafeMutablePointer<statvfs>) -> Int32 {
        return 0
    }
    
    func os_readdir(path: String, buf: UnsafeMutablePointer<Void>, filler: fuse_fill_dir_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
        return 0
    }
    
    func os_mknod(path: String, mode: mode_t, dev: dev_t) -> Int32 {
        return 0
    }
    
    func os_create(path: String, mode: mode_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
        return 0
    }
    
    func os_open(path: String, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
        return 0
    }
    
    
    func os_read(path: String, buf: UnsafeMutablePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
        return 0
    }
    
    
    func os_write(path: String, buf: UnsafePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
        return 0
    }
    
    
    func os_release(path: String, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
        return 0
    }
    
    
    func os_rename(oldpath oldpath: String, newpath: String) -> Int32 {
        return 0
    }
    
    
    func os_unlink(path: String) -> Int32 {
        return 0
    }
    
    
    func os_access(path: String, amode: Int32) -> Int32 {
        return 0
    }
    
    
    func os_rmdir(path: String) -> Int32 {
        return 0
    }
    
    
    func os_mkdir(path: String, mode: mode_t) -> Int32 {
        return 0
    }
    
    
    func os_chmod(path: String, mode: mode_t) -> Int32 {
        return 0
    }
    
    
    func os_chown(path: String, uid: uid_t, gid: gid_t) -> Int32 {
        return 0
    }
    
    
    func os_truncate(path: String, length: off_t) -> Int32 {
        return 0
    }
    
    
    func os_utimens(path: String, tv: UnsafePointer<timespec>) -> Int32 {
        return 0
    }
    
    
    func os_setxattr(path: String, name: UnsafePointer<Int8>, value: UnsafePointer<Int8>, size: size_t, options: Int32, position: UInt32) -> Int32 {
        return 0
    }
    
    
    func os_getxattr(path: String, name: UnsafePointer<Int8>, value: UnsafeMutablePointer<Int8>, size: size_t, position: UInt32) -> Int32 {
        return 0
    }
    
    
    func os_listxattr(path: String, namebuf: UnsafeMutablePointer<Int8>, size: size_t) -> Int32 {
        return 0
    }
    
    
    func os_removexattr(path: String, name: UnsafePointer<Int8>) -> Int32 {
        return 0
    }

}




// MARK: - Volume Management

extension MuxVolume {
    
    /**
     Returns the next available unique volume index (thread-safe)
     
     - returns: The next available unique volume index
     */
    private static func nextVolumeIndex() -> Int64 {
        var next: Int64!
        dispatch_sync(_volumeIndexQueue) {
            next = MuxVolume._nextVolumeIndex
            MuxVolume._nextVolumeIndex++
        }
        return next
    }
    
    /**
     Inserts the volume at a given index (thread-safe)
     
     - parameter volume: The volume to insert
     - parameter index:  The index to insert the volume
     */
    private static func insertVolume(volume: MuxVolume, atIndex index: Int64) {
        dispatch_barrier_sync(_volumeHashQueue) {
            MuxVolume._volumeHash[index] = volume
        }
    }
    
    /**
     Adds a volume to the mux manager
     */
    @inline(__always) private static func addVolumeToHash(newVolume: MuxVolume) {
        insertVolume(newVolume, atIndex: newVolume.volumeIndex)
    }
    
    /**
     Returns the volume at the given volume index (thread-safe)
     
     - returns: The volume indexed by the given index
     */
    @inline(__always) static func volumeAtIndex(index: Int64) -> MuxVolume? {
        var volume: MuxVolume?
        dispatch_sync(_volumeHashQueue) {
            volume = MuxVolume._volumeHash[index]
        }
        return volume
    }
    
}