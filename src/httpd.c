//
//  httpd.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/7/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <jansson.h>

#include "exlog.h"
#include "httpd.h"


/* ------------------ JSON Helpers ------------------- */

json_t *create_status(int code, char *debug_str) {
	json_t *status = json_object();
	json_object_set_new(status, "code", json_integer(code));
	if (debug_str) {
		json_object_set_new(status, "msg",  json_string(debug_str));
	}
	return status;
}

json_t *create_object_with_status(int code, char *debug_str) {
	json_t *obj = json_object();
	json_t *status = create_status(code, debug_str);
	json_object_set_new(obj, "status",  status);
	return obj;
}




/* ------------------ Action Handlers ---------------- */

void handle_add_volume_request(struct MHD_Connection *connection) {
	struct MHD_Response * response;
	json_t *resp = create_object_with_status(2, "fuck");
	char *response_text = json_dumps(resp, JSON_INDENT(4));
	response = MHD_create_response_from_data(strlen(response_text), response_text, MHD_YES, MHD_NO);
	MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
}



/* ------------------ Main Daemon -------------------- */

int httpd_access_handler (void *cls,
						  struct MHD_Connection * connection,
						  const char *url,
						  const char *method,
						  const char *version,
						  const char *upload_data,
						  size_t *upload_data_size,
						  void **con_cls) {
	
	EXLog(COMM, INFO, "httpd_access_handler [%s:%s]", method, url);
	
	/*
	
	if (*upload_data_size) {
		EXLog(COMM, DBG, "upload_data [%ld]: %s", *upload_data_size, upload_data);
	}
	
	int params = MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, NULL, NULL);
	
	EXLog(COMM, INFO, "httpd_access_handler params [%d]", params);
	
	char *val = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "boop");
	if (val) {
		EXLog(COMM, INFO, "val = %s", val);
	}
	 
	 */

	#define URL_MATCHES( _u ) !strncasecmp( url, _u , strlen( _u ))
	
	if (URL_MATCHES( "/add_volume" )) {
		handle_add_volume_request(connection);
	} else {
		EXLog(COMM, DBG, "URL is not a valid command");
		struct MHD_Response * response;
		char *response_text = "Invalid Command";
		response = MHD_create_response_from_data(strlen(response_text), response_text, MHD_NO, MHD_YES);
		MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
		MHD_destroy_response(response);
	}
	
	
	return MHD_YES;
}


void start_httpd_daemon(int port) {
	struct MHD_Daemon * d;

	d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
						 port,
						 NULL,
						 NULL,
						 &httpd_access_handler,
						 NULL,
						 MHD_OPTION_END);
	if (d == NULL) {
		EXLog(COMM, ERR, "http handler did not start [port %d]!", port);
	} else {
		EXLog(COMM, INFO, "http handler started successfully [port %d]", port);
	}
}



