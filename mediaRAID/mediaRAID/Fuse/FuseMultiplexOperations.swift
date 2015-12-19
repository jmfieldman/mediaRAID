//
//  FuseOperations.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/6/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//


/**
Create the FUSE operation struct for use with the MuxVolume

- returns: A fuse_operations struct that can be used with the MuxVolume class
*/
func multiplex_operations() -> fuse_operations {
    
    return fuse_operations.init(
        getattr:        multiplex_getattr,
        readlink:       nil, // No symlink support in mediaRAID
        getdir:         nil, // deprecated (readdir)
        mknod:          multiplex_mknod,
        mkdir:          multiplex_mkdir,
        unlink:         multiplex_unlink,
        rmdir:          multiplex_rmdir,
        symlink:        nil, // No symlink support in mediaRAID
        rename:         multiplex_rename,
        link:           nil, // No hardlink support in mediaRAID
        chmod:          multiplex_chmod,
        chown:          multiplex_chown,
        truncate:       multiplex_truncate,
        utime:          nil, // deprecated (utimens)
        open:           multiplex_open,
        read:           multiplex_read,
        write:          multiplex_write,
        statfs:         multiplex_statfs,
        flush:          nil, // No data is cached by mediaRAID
        release:        multiplex_release,
        fsync:          nil, // No data is cached by mediaRAID
        setxattr:       multiplex_setxattr,
        getxattr:       multiplex_getxattr,
        listxattr:      multiplex_listxattr,
        removexattr:    multiplex_removexattr,
        opendir:        nil, // Not needed since using default_permissions
        readdir:        multiplex_readdir,
        releasedir:     nil, // Not using opendir
        fsyncdir:       nil, // No data is cached by mediaRAID
        `init`:         multiplex_init,
        destroy:        nil, // TBD
        access:         multiplex_access,
        create:         multiplex_create,
        ftruncate:      nil, // Falls back to truncate
        fgetattr:       multiplex_fgetattr,
        lock:           nil, // TODO
        utimens:        multiplex_utimens,
        bmap:           nil, // Not used
        reserved00:     nil,
        reserved01:     nil,
        reserved02:     nil,
        reserved03:     nil,
        reserved04:     nil,
        reserved05:     nil,
        reserved06:     nil,
        reserved07:     nil,
        reserved08:     nil,
        reserved09:     nil,
        reserved10:     nil,
        setvolname:     nil,
        exchange:       nil,
        getxtimes:      nil,
        setbkuptime:    nil,
        setchgtime:     nil,
        setcrtime:      nil,
        chflags:        nil,
        setattr_x:      nil,
        fsetattr_x:     nil)
    
}


var __currentMuxVolumeIndex:        UnsafeMutablePointer<Void>!
var __currentMuxVolumeIndexLock:    OSSpinLock = OS_SPINLOCK_INIT


func multiplex_init(conn: UnsafeMutablePointer<fuse_conn_info>) -> UnsafeMutablePointer<Void> {
    
    let index = __currentMuxVolumeIndex!
    let volumeIndex = unsafeBitCast(index, UnsafeMutablePointer<Int64>.self).memory
    
    OSSpinLockUnlock(&__currentMuxVolumeIndexLock)
    
    if let volume = MuxVolume.volumeAtIndex(volumeIndex) {
        volume.os_initialize()
    }
    
    return index
}


func multiplex_getattr(path: UnsafePointer<Int8>, stbuf: UnsafeMutablePointer<stat>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_getattr(path, stbuf: stbuf)
    
}


func multiplex_fgetattr(path: UnsafePointer<Int8>, stbuf: UnsafeMutablePointer<stat>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_fgetattr(path, stbuf: stbuf, fi: fi)
    
}


func multiplex_statfs(path: UnsafePointer<Int8>, statbuf: UnsafeMutablePointer<statvfs>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_statfs(path, statbuf: statbuf)
    
}


func multiplex_readdir(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Void>, filler: fuse_fill_dir_t!, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return 0
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return 0
    }
    
    return volume.os_readdir(path, buf: buf, filler: filler, offset: offset, fi: fi)
    
}


func multiplex_mknod(path: UnsafePointer<Int8>, mode: mode_t, dev: dev_t) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_mknod(path, mode: mode, dev: dev)
    
}


func multiplex_create(path: UnsafePointer<Int8>, mode: mode_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -ENOENT
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -ENOENT
    }
    
    return volume.os_create(path, mode: mode, fi: fi)
    
}


func multiplex_open(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -ENOENT
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -ENOENT
    }
    
    return volume.os_open(path, fi: fi)
    
}


func multiplex_read(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_read(path, buf: buf, size: size, offset: offset, fi: fi)
    
}


func multiplex_write(path: UnsafePointer<Int8>, buf: UnsafePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_write(path, buf: buf, size: size, offset: offset, fi: fi)
    
}


func multiplex_release(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_release(path, fi: fi)
    
}


func multiplex_rename(oldpath: UnsafePointer<Int8>, newpath: UnsafePointer<Int8>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let oldpath = String.fromCString(oldpath), newpath = String.fromCString(newpath) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_rename(oldpath: oldpath, newpath: newpath)
    
}


func multiplex_unlink(path: UnsafePointer<Int8>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_unlink(path)
    
}


func multiplex_access(path: UnsafePointer<Int8>, amode: Int32) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_access(path, amode: amode)
    
}


func multiplex_rmdir(path: UnsafePointer<Int8>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_rmdir(path)
    
}


func multiplex_mkdir(path: UnsafePointer<Int8>, mode: mode_t) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_mkdir(path, mode: mode)
    
}


func multiplex_chmod(path: UnsafePointer<Int8>, mode: mode_t) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_chmod(path, mode: mode)
    
}


func multiplex_chown(path: UnsafePointer<Int8>, uid: uid_t, gid: gid_t) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_chown(path, uid: uid, gid: gid)
    
}


func multiplex_truncate(path: UnsafePointer<Int8>, length: off_t) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -ENOENT
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -ENOENT
    }
    
    return volume.os_truncate(path, length: length)
    
}


func multiplex_utimens(path: UnsafePointer<Int8>, tv: UnsafePointer<timespec>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_utimens(path, tv: tv)
    
}


func multiplex_setxattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>, value: UnsafePointer<Int8>, size: size_t, options: Int32, position: UInt32) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_setxattr(path, name: name, value: value, size: size, options: options, position: position)
    
}


func multiplex_getxattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>, value: UnsafeMutablePointer<Int8>, size: size_t, position: UInt32) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_getxattr(path, name: name, value: value, size: size, position: position)
    
}


func multiplex_listxattr(path: UnsafePointer<Int8>, namebuf: UnsafeMutablePointer<Int8>, size: size_t) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_listxattr(path, namebuf: namebuf, size: size)
    
}


func multiplex_removexattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>) -> Int32 {
    
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, UnsafeMutablePointer<Int64>.self).memory
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = MuxVolume.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_removexattr(path, name: name)
    
}



