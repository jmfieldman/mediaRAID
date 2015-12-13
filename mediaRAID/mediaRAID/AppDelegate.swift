//
//  AppDelegate.swift
//  mediaRAID
//
//  Created by Jason Fieldman on 12/13/15.
//  Copyright © 2015 Jason Fieldman. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

    @IBOutlet weak var window: NSWindow!

    var statusItem: NSStatusItem!

    /**
     Called when application launches
     
     - parameter aNotification: The launch notification
     */
    func applicationDidFinishLaunching(aNotification: NSNotification) {
        // Insert code here to initialize your application
        
        self.installStatusBarItem()
        
        self.window.releasedWhenClosed = false
        self.window.makeKeyAndOrderFront(self)
    }

    
    /**
     Called when the application will terminate.
     
     - parameter aNotification: The terminate notification
     */
    func applicationWillTerminate(aNotification: NSNotification) {
        // Insert code here to tear down your application
    }
    
    
    /**
     Prevent termination when window red-button is clicked
     
     - parameter sender: The application
     
     - returns: Refuses to terminate the application.
     */
    func applicationShouldTerminate(sender: NSApplication) -> NSApplicationTerminateReply {
        self.window.close()
        return .TerminateCancel
    }


    /**
     Install status bar item for the application
     */
    func installStatusBarItem() {
        
        let bar = NSStatusBar.systemStatusBar()
        statusItem = bar.statusItemWithLength(NSVariableStatusItemLength)
        
        statusItem.image = NSImage(named: "statusbar_off")
        statusItem.highlightMode = true
        
        statusItem.target = self
        statusItem.action = "statusBarIconClicked"        
    }
	
    /**
     Handle status bar icon clicked - show main window
     */
    func statusBarIconClicked() {
        NSApplication.sharedApplication().activateIgnoringOtherApps(true)
        self.window.makeKeyAndOrderFront(self)
    }
    
}

