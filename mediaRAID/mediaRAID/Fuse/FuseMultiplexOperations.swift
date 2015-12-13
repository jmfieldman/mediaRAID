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
    
    var operations = fuse_operations()
    
    operations.getattr      = multiplex_getattr
    operations.fgetattr     = multiplex_fgetattr
    operations.statfs       = multiplex_statfs
    operations.readdir      = multiplex_readdir
    operations.mknod        = multiplex_mknod
    
    /*
.create         = multiplex_create,
.open           = multiplex_open,
.read           = multiplex_read,
.write          = multiplex_write,
.release        = multiplex_release,
.rename         = multiplex_rename,
.unlink         = multiplex_unlink,
.rmdir          = multiplex_rmdir,
.mkdir          = multiplex_mkdir,
.chmod          = multiplex_chmod,
.chown          = multiplex_chown,
.access         = multiplex_access,
.truncate       = multiplex_truncate,
.utimens        = multiplex_utimens,
.setxattr       = multiplex_setxattr,
.getxattr       = multiplex_getxattr,
.listxattr      = multiplex_listxattr,
.removexattr    = multiplex_removexattr,
*/

    return operations
}


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


func multiplex_readdir(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Void>, filler: fuse_fill_dir_t!, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
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


func multiplex_create(path: UnsafePointer<Int8>, mode: mode_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -ENOENT
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -ENOENT
    }
    
    return volume.os_create(path, mode: mode, fi: fi)
    
}


func multiplex_open(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -ENOENT
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -ENOENT
    }
    
    return volume.os_open(path, fi: fi)
    
}


func multiplex_read(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_read(path, buf: buf, size: size, offset: offset, fi: fi)
    
}


func multiplex_write(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Int8>, size: size_t, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_write(path, buf: buf, size: size, offset: offset, fi: fi)
    
}


func multiplex_release(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_release(path, fi: fi)
    
}


func multiplex_rename(oldpath: UnsafePointer<Int8>, newpath: UnsafePointer<Int8>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let oldpath = String.fromCString(oldpath), newpath = String.fromCString(newpath) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_rename(oldpath: oldpath, newpath: newpath)
    
}


func multiplex_unlink(path: UnsafePointer<Int8>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_unlink(path)
    
}


func multiplex_access(path: UnsafePointer<Int8>, amode: Int32) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_access(path, amode: amode)
    
}


func multiplex_rmdir(path: UnsafePointer<Int8>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_rmdir(path)
    
}


func multiplex_mkdir(path: UnsafePointer<Int8>, mode: mode_t) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_mkdir(path, mode: mode)
    
}


func multiplex_chmod(path: UnsafePointer<Int8>, mode: mode_t) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_chmod(path, mode: mode)
    
}


func multiplex_chown(path: UnsafePointer<Int8>, uid: uid_t, gid: gid_t) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_chown(path, uid: uid, gid: gid)
    
}


func multiplex_truncate(path: UnsafePointer<Int8>, length: off_t) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -ENOENT
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -ENOENT
    }
    
    return volume.os_truncate(path, length: length)
    
}


func multiplex_utimens(path: UnsafePointer<Int8>, tv: UnsafePointer<timespec>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = ENOENT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = ENOENT
        return -1
    }
    
    return volume.os_utimens(path, tv: tv)
    
}


func multiplex_setxattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>, value: UnsafePointer<Int8>, size: size_t, options: Int32) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_setxattr(path, name: name, value: value, size: size, options: options)
    
}


func multiplex_getxattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>, value: UnsafeMutablePointer<Int8>, size: size_t) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_getxattr(path, name: name, value: value, size: size)
    
}


func multiplex_listxattr(path: UnsafePointer<Int8>, namebuf: UnsafeMutablePointer<Int8>, size: size_t) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_listxattr(path, namebuf: namebuf, size: size)
    
}


func multiplex_removexattr(path: UnsafePointer<Int8>, name: UnsafePointer<Int8>) -> Int32 {
	
    let volumeIndex = unsafeBitCast(fuse_get_context().memory.private_data, Int64.self)
    
    guard let path = String.fromCString(path) else {
        errno = EFAULT
        return -1
    }
    
    guard let volume = VolumeManager.volumeAtIndex(volumeIndex) else {
        errno = EFAULT
        return -1
    }
    
    return volume.os_removexattr(path, name: name)
    
}



