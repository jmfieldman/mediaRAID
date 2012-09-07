//
//  Header.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/7/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_Header_h
#define mediaRAID_Header_h

#include <microhttpd.h>


/* JSON status error codes */

#define MRAID_OK                                                      0

#define MRAID_ERR_INVALID_PARAM                                       100

#define MRAID_ERR_VOLUME_ALREADY_EXISTS                               200

/* Start the daemon */
void start_httpd_daemon(int port);


#endif
