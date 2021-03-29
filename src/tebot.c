#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h>
#include "tebot.h"

#define LOG_LEVEL_CRITICAL_STRING          "[CRITICAL]: "
#define LOG_LEVEL_NOTICE_STRING            "[NOTICE]: "
#define LOG_LEVEL_BAD_REQUEST_STRING       "[BAD REQUEST]: "

static void log_time ( const tebot_log_level_enum log_level, const char *log_file, const int show_debug, const char *fmt, ... ) {
	FILE *fp = NULL;

	if ( log_file ) {
		if ( access ( log_file, F_OK ) ) fp = fopen ( log_file, "w" );
		else fp = fopen ( log_file, "a" );
		if ( !fp ) return;
	}

	va_list ap;
	char *p;
	int size = 0;

	va_start ( ap, fmt );
	size = vsnprintf ( p, size, fmt, ap );
	va_end ( ap );

	if ( size < 0 ) {
		fclose ( fp );
		return;
	}

	size++;
	p = calloc ( size, 1 );
	if ( !p ) {
		fclose ( fp );
		return;
	}

	va_start ( ap, fmt );
	size = vsnprintf ( p, size, fmt, ap );
	va_end ( ap );

	if ( size < 0 ) {
		free ( p );
		fclose ( fp );
		return;
	}

	time_t current_time = time ( NULL );

	char *level = NULL;

	if ( log_file ) {
		switch ( log_level ) {
			case LOG_LEVEL_CRITICAL:
				fprintf ( fp, LOG_LEVEL_CRITICAL_STRING "%s: %s\n", ctime ( &current_time ), p );
				break;
			case LOG_LEVEL_NOTICE:
				fprintf ( fp, LOG_LEVEL_NOTICE_STRING "%s: %s\n", ctime ( &current_time ), p );
				break;
			case LOG_LEVEL_BAD_REQUEST:
				fprintf ( fp, LOG_LEVEL_BAD_REQUEST_STRING "%s: %s\n", ctime ( &current_time ), p );
				break;
		}
	}

	if ( show_debug ) {
		switch ( log_level ) {
			case LOG_LEVEL_CRITICAL:
				fprintf ( stderr, LOG_LEVEL_CRITICAL_STRING "%s", p );
				break;
			case LOG_LEVEL_NOTICE:
				fprintf ( stderr, LOG_LEVEL_NOTICE_STRING "%s", p );
				break;
			case LOG_LEVEL_BAD_REQUEST:
				fprintf ( stderr, LOG_LEVEL_BAD_REQUEST_STRING "%s", p );
				break;
		}
	}

	free ( p );
	fclose ( fp );
}

tebot_handler_t *tebot_init ( const char *token, tebot_show_debug_enum show_debug, const char *log_file ) {

	tebot_handler_t *h = calloc ( 1, sizeof ( tebot_handler_t ) );
	if ( !h ) {
		if ( log_file ) {
			log_time ( LOG_LEVEL_CRITICAL, log_file, show_debug, "Not enought memory for alloc to tebot handler.\n" );
		}
		return NULL;
	}

	h->url_get = calloc ( 4097, 1 );
	if ( !h->url_get ) {
		if ( log_file ) {
			log_time ( LOG_LEVEL_CRITICAL, log_file, show_debug, "Not enought memory for alloc to tebot url get.\n" );
		}
		free ( h );
		return NULL;
	}

	h->current_buf = calloc ( 4097, 1 );
	if ( !h->current_buf ) {
		if ( log_file ) {
			log_time ( LOG_LEVEL_CRITICAL, log_file, show_debug, "Not enought memory for alloc to tebot current buf.\n" );
		}
		free ( h->url_get );
		free ( h );
		return NULL;
	}

	const int size_of_token = strlen ( token );
	h->token = calloc ( size_of_token + 1, 1 );
	if ( !h->token ) {
		if ( log_file ) {
			log_time ( LOG_LEVEL_CRITICAL, log_file, show_debug, "Not enought memory for alloc to tebot token.\n" );
		}
		free ( h->current_buf );
		free ( h->url_get );
		free ( h );
		return NULL;
	}
	strncpy ( h->token, token, size_of_token );

	h->show_debug = show_debug;

	h->curl = curl_easy_init ( );

	return h;
}

static size_t write_data_cb ( void *data, size_t size, size_t nmemb, void *st ) {
	tebot_handler_t *h = ( tebot_handler_t * ) st;

	size_t size_of_data = size * nmemb;
	char *current_buf = ( char * ) realloc ( h->current_buf, h->offset + size_of_data + 1 );
	if ( !current_buf ) {
		log_time ( LOG_LEVEL_CRITICAL, h->log_file, h->show_debug, "failed to allocate memory for size: %u\n", size_of_data );
		return 0;
	}

	memcpy ( &current_buf [ h->offset ], data, size_of_data );
	h->current_buf = current_buf;
	h->offset += size_of_data;
	h->current_buf [ h->offset ] = 0;

	return size_of_data;
}

static unsigned char *tebot_request_get ( tebot_handler_t *h, const char *method, void *d ) {

	snprintf ( h->url_get, 4097, "%s/bot%s/%s", URL_API, h->token, method );

	h->offset = 0;
	
	curl_easy_setopt ( h->curl, CURLOPT_URL, h->url_get );
	curl_easy_setopt ( h->curl, CURLOPT_WRITEFUNCTION, write_data_cb );
	curl_easy_setopt ( h->curl, CURLOPT_WRITEDATA, h );

	CURLcode res;
	res = curl_easy_perform ( h->curl );
	if ( res != CURLE_OK ) {
		log_time ( LOG_LEVEL_BAD_REQUEST, h->log_file, h->show_debug, "failed to request get - %s (%d)\n", curl_easy_strerror ( res ), res );
	}

	long response_code = 0L;
	curl_easy_getinfo ( h->curl, CURLINFO_RESPONSE_CODE, &response_code );
	if ( response_code != 200L ) {
		log_time ( LOG_LEVEL_NOTICE, h->log_file, h->show_debug, "answer from server: %d\n", response_code );
	}

	curl_easy_cleanup ( h->curl );

	return h->current_buf;
}

struct data_of_types {
	char *name;
	void **ptr;
};

static int parse_data ( tebot_handler_t *h, char *data, struct data_of_types dot[], const int size ) {

	json_object *root = NULL;
	json_object *ok = NULL;
	json_object *json_result = NULL;
	json_object *param = NULL;

	root = json_tokener_parse ( data );
	if ( !root ) return -1;

	ok = json_object_object_get ( root, "ok" );
	json_type type = json_object_get_type ( ok );
	if ( type != json_type_boolean ) goto parse_data_error;
	json_bool ok_bool = json_object_get_boolean ( ok );
	if ( ok_bool != 1 ) goto parse_data_error;

	json_result = json_object_object_get ( root, "result" );
	if ( !json_result ) {
		goto parse_data_error;
	}
	type = json_object_get_type ( json_result );
	if ( type != json_type_object ) goto parse_data_error;

	for ( int i = 0; i < size; i++ ) {
		param = json_object_object_get ( json_result, dot[i].name );
		if ( !param ) {
			log_time ( LOG_LEVEL_NOTICE, h->log_file, h->show_debug, "failed. not found name of param: %s\n", dot[i].name );
			continue;
		}

		json_type type = json_object_get_type ( param );
		switch ( type ) {
			case json_type_null:
				*dot[i].ptr = NULL;
				break;
			case json_type_boolean:
				*( ( unsigned char * ) dot[i].ptr ) = 1;
				break;
			case json_type_int:
				*( ( long long int * ) dot[i].ptr ) = json_object_get_int64 ( param );
				break;
			case json_type_double:
				*( ( double * ) dot[i].ptr ) = json_object_get_double ( param );
				break;
			case json_type_string: {
				const char *str = json_object_get_string ( param );
				const int length = strlen ( str );
				*dot[i].ptr = ( char * ) calloc ( length + 1, 1 );
				strncpy ( *dot[i].ptr, str, length );
		 		}
				break;
			default:
				break;
		}
	}

	json_object_put ( root );

	return 0;
parse_data_error:
	if ( root ) json_object_put ( root );
	return -1;
}

tebot_user_t *tebot_method_get_me ( tebot_handler_t *h ) {

	char *data = tebot_request_get ( h, "getMe", NULL );

	tebot_user_t *user = calloc ( 1, sizeof ( tebot_user_t ) );

	struct data_of_types dot[] = {
		{ "id", (void **) &user->id },
		{ "is_bot", (void **) &user->is_bot },
		{ "first_name", (void **) &user->first_name },
		{ "username", (void **) &user->username },
		{ "can_join_groups", (void **) &user->can_join_groups },
		{ "can_read_all_group_messages", (void **) &user->can_read_all_group_messages },
		{ "supports_inline_queries", (void **) &user->supports_inline_queries }
	};

	int ret = parse_data ( h, data, dot, 7 );
	if ( ret == -1 ) {
		log_time ( LOG_LEVEL_NOTICE, h->log_file, h->show_debug, "failed to parse data: %s\n", data );
	}

	return user;
}
