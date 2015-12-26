//
//  ReplicationQueue.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/26/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Foundation

/// This is a simple circular buffer implemented over an array
class ReplicationQueue {
    
    /** The minimum size of the buffer */
    private static let MinSize      = 256
    private static let MaxIncrement = 32768
    
    /** The operation buffer */
    private var queue = Array<ReplicationOp?>(count: ReplicationQueue.MinSize, repeatedValue: nil)
    
    /* Buffer indexes */
    private var front = 0
    private var back  = 0
    
    /** Read/write lock queue */
    private let dispatch_queue = dispatch_queue_create("mediaRAID.ReplicationQueue", DISPATCH_QUEUE_CONCURRENT)
    
    
    func count() -> Int {
        var r: Int!
        dispatch_sync(dispatch_queue) {
            r = self._count()
        }
        return r
    }
    
    func isEmpty() -> Bool {
        var r: Bool!
        dispatch_sync(dispatch_queue) {
            r = self._isEmpty()
        }
        return r
    }
    
    func pushFront(op: ReplicationOp) {
        dispatch_barrier_sync(dispatch_queue) {
            self.front = self.front - 1
            if (self.front < 0) {
                self.front += self.queue.count
            }
            self.queue[self.front] = op
            if (self._count() == (self.queue.count - 1)) {
                self._resize()
            }
        }
    }
    
    func pushBack(op: ReplicationOp) {
        dispatch_barrier_sync(dispatch_queue) {
            self.back = self.back + 1
            if (self.back == self.queue.count) {
                self.back = 0
            }
            self.queue[self.back] = op
            if (self._count() == (self.queue.count - 1)) {
                self._resize()
            }
        }
    }
    
    func popFront() -> ReplicationOp? {
        var r: ReplicationOp?
        dispatch_barrier_sync(dispatch_queue) {
            if (self._isEmpty()) {
                return
            }
            r = self.queue[self.front]
            self.front = self.front + 1
            if (self._count() < (self.queue.count / 8)) {
                self._resize()
            }
        }
        return r
    }
    
    
    private func _count() -> Int {
        return (front <= back) ? (back - front) : (queue.count - front + back)
    }
    
    private func _isEmpty() -> Bool {
        return front == back
    }
    
    private func _resize() {
        let curCount = self._count()
        let newsize = max(curCount + min(curCount, ReplicationQueue.MaxIncrement), ReplicationQueue.MinSize)
        
        
        var newqueue: [ReplicationOp?] = []
        if (front <= back) {
            newqueue.appendContentsOf( queue.dropLast(queue.count - back).dropFirst(front) )
        } else {
            newqueue.appendContentsOf( queue.dropFirst(front) )
            newqueue.appendContentsOf( queue.dropLast(back)   )
        }
        
        newqueue.appendContentsOf( Array<ReplicationOp?>(count: max(newsize - newqueue.count, 0), repeatedValue: nil))
        
        front = 0
        back  = curCount
        queue = newqueue
    }
    
    
    
}