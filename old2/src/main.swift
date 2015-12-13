//
//  main.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/6/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//


print("Hello, World!")

var ararr = Process.arguments
ararr += ["-o", "default_permissions", "-o", "allow_other"]

print("args: \(ararr) | \(Process.arguments)")

var cargs = ararr.map { strdup($0) }

print("blah: \(cargs)")

var args: fuse_args = fuse_args(argc: Int32(cargs.count), argv: &cargs, allocated: 0)

//for ptr in cargs { free(ptr) }


var fuse_oper_struct: fuse_operations = fuse_operations()

/*
func multiplex_statfs(foo: UnsafePointer<Int8>, statfs: UnsafeMutablePointer<statvfs>) -> Int32 {
	
	let pl: String? = String.fromCString(foo)
	let s:  statvfs = statfs.memory
	
	if (statfs == nil) {
		
	}
	
	return 0
}
	


fuse_oper_struct.statfs = multiplex_statfs
//fuse_oper_struct.init   = multiplex_init
fuse_oper_struct.getattr = multiplex_getattr
fuse_oper_struct.getattr = multiplex_getattr


fuse_main_real(args.argc, args.argv, &fuse_oper_struct, sizeof(fuse_operations), nil)

*/



let hello_str = "Hello World!\n"
let hello_path = "/hello"

func hello_getattr(path: UnsafePointer<Int8>, stbuf: UnsafeMutablePointer<stat>) -> Int32 {
	var res: Int32 = 0;
	
	let pt = String.fromCString(path)
	print("getattr: \(pt)")
	
	stbuf.memory = stat()
	
	if (strcmp(path, "/") == 0) {
		stbuf.memory.st_mode = S_IFDIR | 0755;
		stbuf.memory.st_nlink = 2;
		print("$$")
	} else if (strcmp(path, hello_path) == 0) {
		stbuf.memory.st_mode = S_IFREG | 0444;
		stbuf.memory.st_nlink = 1;
		stbuf.memory.st_size = Int64(hello_str.utf8.count)
		print("$$$")
	} else {
		res = -ENOENT
		print("$")
	}
	
	return res
}

func hello_readdir(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Void>, filler: fuse_fill_dir_t!,
	offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	
	print("trying to read path: \(path)")
		
	if (strcmp(path, "/") != 0) {
		return -ENOENT
	}
	
	filler(buf, ".", nil, 0);
	filler(buf, "..", nil, 0);
	filler(buf, "hello", nil, 0);
	
	return 0;
}

func hello_open(path: UnsafePointer<Int8>, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {
	print("open")
	
	if (strcmp(path, hello_path) != 0) {
		return -ENOENT;
	}
	
	if ((fi.memory.flags & 3) != O_RDONLY) {
		return -EACCES;
	}
	
	return 0;
}

func hello_read(path: UnsafePointer<Int8>, buf: UnsafeMutablePointer<Int8>, size: Int, offset: off_t, fi: UnsafeMutablePointer<fuse_file_info>) -> Int32 {

	print("read")
	
	var len: size_t = 0
	var size = size
	
	if(strcmp(path, hello_path) != 0) {
		return -ENOENT;
	}
	
	len = size_t(hello_str.utf8.count)
	if (size_t(offset) < len) {
		if (size_t(offset) + size > len) {
			size = len - size_t(offset);
		}
		memcpy(buf, hello_str, size);
	} else {
		size = 0;
	}
	
	return Int32(size);
}

var hello_oper: fuse_operations = fuse_operations()
hello_oper.getattr = hello_getattr
hello_oper.readdir = hello_readdir
hello_oper.open = hello_open
hello_oper.read = hello_read


var ch = fuse_mount("/tmp/fusefoo", &args)
print("ch: \(ch)")

let se = fuse_new(ch, &args, &hello_oper, sizeof(fuse_operations), nil)
print("se: \(se)")

let sighand = fuse_set_signal_handlers(se)
print("sig: \(sighand)")

//fuse_session_add_chan(se, ch)

let err = fuse_loop(se)

//let err = fuse_session_loop(se);
print("err: \(err)")

//sleep(3)

fuse_remove_signal_handlers(se)
fuse_session_destroy(se)

fuse_unmount("/tmp/fusefoo", ch)

print("done!")
