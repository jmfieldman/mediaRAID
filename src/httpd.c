//
//  httpd.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/7/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <jansson.h>

#include "exlog.h"
#include "httpd.h"
#include "volumes.h"

/* ------------------ JSON Helpers ------------------- */

/* Creates a status dictionary */
json_t *create_status(int code, char *debug_str) {
	json_t *status = json_object();
	json_object_set_new(status, "code", json_integer(code));
	if (debug_str) {
		json_object_set_new(status, "msg",  json_string(debug_str));
	}
	return status;
}

/* Creates a top-level object with the given status child */
json_t *create_object_with_status(int code, char *debug_str) {
	json_t *obj = json_object();
	json_t *status = create_status(code, debug_str);
	json_object_set_new(obj, "status",  status);
	return obj;
}


/* ------------------ Response Helpers --------------- */

/* Handles the conversion from JSON->text and sending the response, then decrefs the object */
void send_json_response(struct MHD_Connection *connection, unsigned int http_code, json_t *json_object) {
	struct MHD_Response * response;
	char *response_text = json_dumps(json_object, JSON_PRESERVE_ORDER | JSON_INDENT(4));
	response = MHD_create_response_from_data(strlen(response_text), response_text, MHD_YES, MHD_NO);
	MHD_queue_response(connection, http_code, response);
	MHD_destroy_response(response);
	json_decref(json_object);
}


/* ------------------ Directory Helpers -------------- */

/* Put source->dest.  Ensure leading slash, ensure no trailing slash.  dest must be pre-allocated */
static void cleandir(const char *source, char *dest) {
	if (!source || *source == 0) {
		*dest = 0;
		return;
	}
	
	/* Add leading slash */
	if (*source == '/') {
		strncpy(dest, source, PATH_MAX);
	} else {
		snprintf(dest, PATH_MAX, "/%s", source);
	}
	
	/* Remove trailing slash */
	size_t len = strnlen(dest, PATH_MAX);
	if (dest[len-1] == '/') {
		dest[len-1] = 0;
	}
}


/* ------------------ Action Handlers ---------------- */

/* 
 Adds the volume with the following GET parameters:
 basepath: required; the path to the volume
 alias: optional; alias for the volume
 raidpath: optional; custom directory appendix for the raid files
 trashpath: optional; custom directory appendix for the trash files
 */
void handle_volume_add_request(struct MHD_Connection *connection) {
	
	/* Get arguments */
	const char *basepath = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "basepath");
	if (!basepath || !strnlen(basepath, PATH_MAX)) {
		send_json_response(connection, MHD_HTTP_OK, create_object_with_status(MRAID_ERR_INVALID_PARAM, "basepath parameter is required"));
		return;
	}

	const char *alias     = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "alias");
	if (alias && *alias == 0) {
		alias = basepath;
	}
	
	const char *raiddirp  = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "raiddir");
	const char *trashdirp = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "trashdir");

	char raiddir[PATH_MAX];
	char trashdir[PATH_MAX];
	
	cleandir(raiddirp, raiddir);
	cleandir(trashdirp, trashdir);
	
	/* Check volume exists */
	volume_api_lock();
	if (volume_with_basepath(basepath)) {
		volume_api_unlock();
		send_json_response(connection, MHD_HTTP_OK, create_object_with_status(MRAID_ERR_VOLUME_ALREADY_EXISTS, "volume already exists with the specified basepath"));
		return;
	}
	
	if (volume_with_alias(alias)) {
		volume_api_unlock();
		send_json_response(connection, MHD_HTTP_OK, create_object_with_status(MRAID_ERR_VOLUME_ALREADY_EXISTS, "volume already exists with the specified alias"));
		return;
	}

	/* Create the volume */
	RaidVolume_t *volume = create_volume(alias, basepath, raiddir, trashdir);
	if (!volume) {
		volume_api_unlock();
		send_json_response(connection, MHD_HTTP_OK, create_object_with_status(MRAID_ERR_VOLUME_ALLOC_ERROR, "volume could not be allocated"));
		return;
	}
	
	/* Add volume to list */
	set_volume_active(volume, 1);
	
	/* Respond */
	json_t *resp = create_object_with_status(MRAID_OK, "volume added successfully");
	json_object_set_new(resp, "volume", volume_json_object(volume));
	send_json_response(connection, MHD_HTTP_OK, resp);
	
	volume_api_unlock();
}

/*
 Removes the volume with the following GET parameters:
 basepath: removes the volume with this basepath [uses this if both params are present]
 alias: removes the volume with this alias
 */
void handle_volume_remove_request(struct MHD_Connection *connection) {
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
	
	if (URL_MATCHES( "/volume/add" )) {
		handle_volume_add_request(connection);
	} else if (URL_MATCHES( "/volume/remove" )) {
		handle_volume_remove_request(connection);
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



