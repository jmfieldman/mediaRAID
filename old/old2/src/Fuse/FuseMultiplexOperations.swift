//
//  FuseOperations.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/6/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//




func multiplex_init(conn: UnsafeMutablePointer<fuse_conn_info>) {

	let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
	
	guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
		return
	}
	
	volume.os_initialize()
}


func multiplex_getattr(path: UnsafePointer<Int8>, stbuf: UnsafeMutablePointer<stat>) -> Int32 {
	
	let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
	
	guard let path = String.fromCString(path) else {
		errno = ENOENT
		return -1
	}
	
	guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
		errno = ENOENT
		return -1
	}
	
	return volume.os_getattr(path, stbuf: stbuf)
	
}


func multiplex_fgetattr(path: UnsafePointer<Int8>, stbuf: UnsafeMutablePointer<stat>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
	let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
	
	guard let path = String.fromCString(path) else {
		errno = ENOENT
		return -1
	}
	
	guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
		errno = ENOENT
		return -1
	}
	
	return volume.os_fgetattr(path, stbuf: stbuf, fi: fi)
	
}


func multiplex_statfs(path: UnsafePointer<Int8>, statbuf: UnsafeMutablePointer<statvfs>) -> Int32 {
	
	let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
	
	guard let path = String.fromCString(path) else {
		errno = ENOENT
		return -1
	}
	
	guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
		errno = ENOENT
		return -1
	}
	
	return volume.os_statfs(path, statbuf: statbuf)
	
}


func multiplex_readdir(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Void>, filler: fuse_fill_dir_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
	let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
	
	guard let path = String.fromCString(path) else {
		errno = ENOENT
		return 0
	}
	
	guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
		errno = ENOENT
		return 0
	}
	
	return volume.os_readdir(path, buf: buf, filler: filler, offset: offset, fi: fi)
	
}


func multiplex_mknod(path: UnsafePointer<Int8>, mode: mode_t, dev: dev_t) -> Int32 {
	
	let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
	
	guard let path = String.fromCString(path) else {
		errno = ENOENT
		return -1
	}
	
	guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
		errno = ENOENT
		return -1
	}
	
	return volume.os_mknod(path, mode: mode, dev: dev)
	
}


func multiplex_create(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	return 0
}


func multiplex_open(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	return 0
}


func multiplex_read(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	return 0
}


func multiplex_write(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	return 0
}


func multiplex_release(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	return 0
}


func multiplex_rename(oldpath: UnsafePointer<Int8>, newpath: UnsafePointer<Int8>) -> Int32 {
	return 0
}


func multiplex_unlink(path: UnsafePointer<Int8>) -> Int32 {
	return 0
}


func multiplex_access(path: UnsafePointer<Int8>, amode: Int32) -> Int32 {
	return 0
}


func multiplex_rmdir(path: UnsafePointer<Int8>) -> Int32 {
	return 0
}


func multiplex_mkdir(path: UnsafePointer<Int8>, mode: mode_t) -> Int32 {
	return 0
}


func multiplex_chmod(path: UnsafePointer<Int8>, mode: mode_t) -> Int32 {
	return 0
}


func multiplex_chown(path: UnsafePointer<Int8>, uid: uid_t, gid: gid_t) -> Int32 {
	return 0
}


func multiplex_truncate(path: UnsafePointer<Int8>, length: off_t) -> Int32 {
	return 0
}


func multiplex_utimens(path: UnsafePointer<Int8>, tv: UnsafePointer<timespec>) -> Int32 {
	return 0
}


func multiplex_setxattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>, value: UnsafePointer<Int8>, size: size_t, options: Int32) -> Int32 {
	return 0
}


func multiplex_getxattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>, value: UnsafeMutablePointer<Int8>, size: size_t) -> Int32 {
	return 0
}


func multiplex_listxattr(path: UnsafePointer<Int8>, namebuf: UnsafeMutablePointer<Int8>, size: size_t) -> Int32 {
	return 0
}


func multiplex_removexattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>) -> Int32 {
	return 0
}



