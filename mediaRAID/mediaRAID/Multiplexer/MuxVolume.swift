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
}