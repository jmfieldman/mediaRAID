//
//  VolumeManager.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/12/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation


class VolumeManager {
	
	private static var volumeHashQueue  = dispatch_queue_create("mediaRAID.VolumeManager.volumeHashQueue",  DISPATCH_QUEUE_CONCURRENT)
	private static var volumeIndexQueue = dispatch_queue_create("mediaRAID.VolumeManager.volumeIndexQueue", DISPATCH_QUEUE_CONCURRENT)
	
	private static var volumeHash: [Int64 : MuxVolume] = [:]
	private static var _nextVolumeIndex: Int64 = 0
	
	
	private static func nextVolumeIndex() -> Int64 {
		var next: Int64!
		dispatch_sync(volumeIndexQueue) {
			next = VolumeManager._nextVolumeIndex
			VolumeManager._nextVolumeIndex++
		}
		return next
	}
	
	private static func insertVolume(volume: MuxVolume, atIndex index: Int64) {
		dispatch_barrier_sync(volumeHashQueue) {
			VolumeManager.volumeHash[index] = volume
		}
	}
	
	static func addVolume(newVolume: MuxVolume) -> Int64 {
		let index = nextVolumeIndex()
		insertVolume(newVolume, atIndex: index)
		return index
	}
	
	static func volumeAtIndex(index: Int64) -> MuxVolume? {
		var volume: MuxVolume?
		dispatch_sync(volumeHashQueue) {
			volume = VolumeManager.volumeHash[index]
		}
		return volume
	}
	
}