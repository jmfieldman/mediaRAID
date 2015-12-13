//
//  MuxVolume.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/10/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation


class MuxVolume {
	

	
	
	func os_initialize() {
		
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