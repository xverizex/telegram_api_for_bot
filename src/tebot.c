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

#define MIMES_TYPE_PARAM                   0
#define MIMES_TYPE_ARRAY                   1
struct mimes {
	int type;
	char *name;
	char *value;
	char **array;
};

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
	if ( log_file ) fclose ( fp );
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

static unsigned char *tebot_request_get ( tebot_handler_t *h, const char *method, struct mimes *mimes, const int size_mimes ) {

	snprintf ( h->url_get, 4097, "%s/bot%s/%s", URL_API, h->token, method );

	h->curl = curl_easy_init ( );
	h->offset = 0;
	
	curl_easy_setopt ( h->curl, CURLOPT_URL, h->url_get );
	curl_easy_setopt ( h->curl, CURLOPT_WRITEFUNCTION, write_data_cb );
	curl_easy_setopt ( h->curl, CURLOPT_WRITEDATA, h );

	curl_mime *mime = NULL;

	if ( mimes != NULL && size_mimes > 0 ) {
		mime = curl_mime_init ( h->curl );
		
		for ( int i = 0; i < size_mimes; i++ ) {
			curl_mimepart *part = curl_mime_addpart ( mime );
			if ( !part ) {
				log_time ( LOG_LEVEL_CRITICAL, h->log_file, h->show_debug, "failed to create mime part.\n" );
				goto tebot_request_get_error;
			}

			curl_mime_name ( part, mimes[i].name );
			curl_mime_data ( part, mimes[i].value, CURL_ZERO_TERMINATED );
		}
	
		curl_easy_setopt ( h->curl, CURLOPT_MIMEPOST, mime );
	}

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

	if ( mime ) curl_mime_free ( mime );
	curl_easy_cleanup ( h->curl );

	return h->current_buf;

tebot_request_get_error:
	curl_mime_free ( mime );
	curl_easy_cleanup ( h->curl );

	return NULL;
}

struct data_of_types {
	char *name;
	void **ptr;
	int size;
	void (*set_links_for_object_and_get_data) ( tebot_handler_t *h, void *ptr, json_object *param );
};
void parse_current_object ( tebot_handler_t *h, json_object *json_result, struct data_of_types dot[], const int i ) {
	printf ( "parse_current_object.\n" );
	json_object *param = NULL;
	json_object_iter iter;

	param = json_object_object_get ( json_result, dot[i].name );
	if ( !param ) {
		return;
	}

	json_type type = json_object_get_type ( param );
	switch ( type ) {
		case json_type_null:
			*dot[i].ptr = NULL;
			break;
		case json_type_object: 
			if ( dot[i].size <= 0 ) break;
			*( dot[i].ptr ) = calloc ( 1, dot[i].size );
			dot[i].set_links_for_object_and_get_data ( h, *dot[i].ptr, param );
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

static int parse_data ( tebot_handler_t *h, char *data, struct data_of_types dot[], const int size, const int index_of_array ) {
	printf ( "parse_data\n" );

	json_object *root = NULL;
	json_object *ok = NULL;
	json_object *json_result = NULL;

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
	if ( type != json_type_object && type != json_type_array ) goto parse_data_error;

	if ( type == json_type_array ) {
		size_t array_length = json_object_array_length ( json_result );
		if ( index_of_array >= array_length ) return 1;

		if ( index_of_array >= 0 && index_of_array < array_length ) {
			json_object *obj = json_object_array_get_idx ( json_result, index_of_array );
			for ( int i = 0; i < size; i++ ) {
				parse_current_object ( h, obj, dot, i );
			}
		} else {
			for ( int index = 0; index < array_length; index++ ) {
				json_object *obj = json_object_array_get_idx ( json_result, index );

				for ( int i = 0; i < size; i++ ) {
					parse_current_object ( h, json_result, dot, i );
				}
			}
		}

	} else if ( type == json_type_object ) {
		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, json_result, dot, i );
		}
	}


	json_object_put ( root );

	return 0;
parse_data_error:
	if ( root ) json_object_put ( root );
	return -1;
}

tebot_user_t *tebot_method_get_me ( tebot_handler_t *h ) {

	char *data = tebot_request_get ( h, "getMe", NULL, 0 );

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

	int ret = parse_data ( h, data, dot, 7, -1 );
	if ( ret == -1 ) {
		log_time ( LOG_LEVEL_NOTICE, h->log_file, h->show_debug, "failed to parse data: %s\n", data );
	}

	return user;
}

void *strdup_printf ( char *fmt, ... ) {
	va_list ap;
	char *p;
	int size = 0;

	va_start ( ap, fmt );
	size = vsnprintf ( p, size, fmt, ap );
	va_end ( ap );

	if ( size < 0 ) return NULL;

	size++;
	p = calloc ( size, 1 );
	if ( !p ) return NULL; 

	va_start ( ap, fmt );
	size = vsnprintf ( p, size, fmt, ap );
	va_end ( ap );

	if ( size < 0 ) {
		free ( p );
		return NULL;
	}

	return p;
}

void handler_template ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_template_t *obj = ( tebot_template_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "id", (void **) &obj->id },
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_venue ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_venue_t *obj = ( tebot_venue_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "location", (void **) &obj->location, sizeof ( tebot_location_t ), handler_location },
			{ "title", (void **) &obj->title },
			{ "address", (void **) &obj->address },
			{ "foursquare_id", (void **) &obj->foursquare_id },
			{ "foursquare_type", (void **) &obj->foursquare_type },
			{ "google_place_id", (void **) &obj->google_place_id },
			{ "google_place_type", (void **) &obj->google_place_type }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_poll_option ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_poll_option_t *obj = ( tebot_poll_option_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "text", (void **) &obj->text },
			{ "voter_count", (void **) &obj->voter_count }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_poll ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_poll_t *obj = ( tebot_poll_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "id", (void **) &obj->id },
			{ "question", (void **) &obj->question },
			{ "options", (void **) &obj->options, sizeof ( tebot_poll_options_t ), handler_poll_options },
			{ "total_voter_count", (void **) &obj->total_voter_count },
			{ "is_closed", (void **) &obj->is_closed },
			{ "is_anonymous", (void **) &obj->is_anonymous },
			{ "type", (void **) &obj->type },
			{ "allows_multiple_answers", (void **) &obj->allows_multiple_answers },
			{ "correct_option_id", (void **) &obj->correct_option_id },
			{ "explanation", (void **) &obj->explanation },
			{ "explanation_entities", (void **) &obj->explanation_entities, sizeof ( tebot_message_entity_t ),
				handler_message_entity },
			{ "open_period", (void **) &obj->open_period },
			{ "close_date", (void **) &obj->close_date }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_game ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_game_t *obj = ( tebot_game_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "title", (void **) &obj->title },
			{ "description", (void **) &obj->description },
			{ "photo", (void **) &obj->photo, sizeof ( tebot_photo_size_t ), handler_photo_size },
			{ "text", (void **) &obj->text },
			{ "text_entities", (void **) &obj->text_entities, sizeof ( tebot_text_entities_t ), handler_text_entities },
			{ "animation", (void **) &obj->animation, sizeof ( tebot_animation_t ), handler_animation }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_dice ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_dice_t *obj = ( tebot_dice_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "emoji", (void **) &obj->emoji },
			{ "value", (void **) &obj->value }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_contact ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_contact_t *obj = ( tebot_contact_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "phone_number", (void **) &obj->phone_number },
			{ "first_name", (void **) &obj->first_name },
			{ "last_name", (void **) &obj->last_name },
			{ "user_id", (void **) &obj->user_id },
			{ "vcard", (void **) &obj->vcard }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_voice ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_voice_t *obj = ( tebot_voice_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &obj->file_id },
			{ "file_unique_id", (void **) &obj->file_unique_id },
			{ "duration", (void **) &obj->duration },
			{ "mime_type", (void **) &obj->mime_type },
			{ "file_size", (void **) &obj->file_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}
void handler_video_note ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_video_note_t *obj = ( tebot_video_note_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &obj->file_id },
			{ "file_unique_id", (void **) &obj->file_unique_id },
			{ "length", (void **) &obj->length },
			{ "duration", (void **) &obj->duration },
			{ "thumb", (void **) &obj->thumb, sizeof ( tebot_photo_size_t ), handler_photo_size },
			{ "file_name", (void **) &obj->file_name },
			{ "file_size", (void **) &obj->file_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}
void handler_video ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_video_t *obj = ( tebot_video_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &obj->file_id },
			{ "file_unique_id", (void **) &obj->file_unique_id },
			{ "width", (void **) &obj->width },
			{ "height", (void **) &obj->height },
			{ "duration", (void **) &obj->duration },
			{ "thumb", (void **) &obj->thumb, sizeof ( tebot_photo_size_t ), handler_photo_size },
			{ "file_name", (void **) &obj->file_name },
			{ "mime_type", (void **) &obj->mime_type },
			{ "file_size", (void **) &obj->file_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_mask_position ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_mask_position_t *obj = ( tebot_mask_position_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "point", (void **) &obj->point },
			{ "x_shift", (void **) &obj->x_shift },
			{ "y_shift", (void **) &obj->y_shift },
			{ "scale", (void **) &obj->scale }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_sticker ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_sticker_t temp* = ( tebot_sticker_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &sticker->file_id },
			{ "file_unique_id", (void **) &sticker->file_unique_id },
			{ "width", (void **) &sticker->width },
			{ "height", (void **) &sticker->height },
			{ "is_animated", (void **) &sticker->is_animated },
			{ "thumb", (void **) &sticker->thumb, sizeof ( tebot_photo_size_t ), handler_photo_size },
			{ "emoji", (void **) &sticker->emoji },
			{ "set_name", (void **) &sticker->set_name },
			{ "mask_position", (void **) &sticker->mask_position, sizeof ( tebot_mask_position_t ), handler_mask_position },
			{ "file_size", (void **) &sticker->file_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_document ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_document_t document* = ( tebot_document_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &document->file_id },
			{ "file_unique_id", (void **) &document->file_unique_id },
			{ "thumb", (void **) &document->thumb, sizeof ( tebot_photo_size_t ), handler_photo_size },
			{ "file_name", (void **) &document->file_name },
			{ "mime_type", (void **) &document->mime_type },
			{ "file_size", (void **) &document->file_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_audio ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_audio_t audio* = ( tebot_audio_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &audio->file_id },
			{ "file_unique_id", (void **) &audio->file_unique_id },
			{ "duration", (void **) &audio->duration },
			{ "performer", (void **) &audio->performer },
			{ "title", (void **) &audio->title },
			{ "file_name", (void **) &audio->file_name },
			{ "mime_type", (void **) &audio->mime_type },
			{ "file_size", (void **) &audio->file_size },
			{ "thumb", (void **) &audio->thumb, sizeof ( tebot_photo_size_t ), handler_photo_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}
void handler_photo_size ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_photo_size_t *photo_size = ( tebot_photo_size_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &photo_size->file_id },
			{ "file_unique_id", (void **) &photo_size->file_unique_id },
			{ "width", (void **) &photo_size->width },
			{ "height", (void **) &photo_size->height },
			{ "file_size", (void **) &photo_size->file_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_animation ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_animation_t *animation = ( tebot_animation_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "file_id", (void **) &animation->file_id },
			{ "file_unique_id", (void **) &animation->file_unique_id },
			{ "width", (void **) &animation->width },
			{ "height", (void **) &animation->height },
			{ "duration", (void **) &animation->duration },
			{ "thumb", (void **) &animation->thumb, sizeof ( tebot_photo_size_t ), handler_photo_size },
			{ "file_name", (void **) &animation->file_name },
			{ "mime_type", (void **) &animation->mime_type },
			{ "file_size", (void **) &animation->file_size }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_message_entity ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_message_entity_t message_entity* = ( tebot_message_entity_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "type", (void **) &message_entity->type },
			{ "offset", (void **) &message_entity->offset },
			{ "length" (void **) &message_entity->length },
			{ "url", (void **) &message_entity->url },
			{ "user", (void **) &message_entity->user, sizeof ( tebot_user_t ), handler_user },
			{ "language", (void **) &message_entity->language }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_location ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_location_t location* = ( tebot_location_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "longitude", (void **) &location->longitude },
			{ "latitude", (void **) &location->latitude },
			{ "horizontal_accuracy", (void **) &location->horizontal_accuracy },
			{ "live_period", (void **) &location->live_period },
			{ "heading", (void **) &location->heading },
			{ "proximity_alert_radius", (void **) &location->proximity_alert_radius }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_chat_location ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_chat_location_t *chat_location = ( tebot_chat_location_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "location", (void **) &chat_location->location, sizeof ( tebot_location_t ),
		       			handler_location	},
			{ "address", (void **) &chat_address }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_chat_permissions ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_template_t chat_permissions* = ( tebot_chat_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "can_send_messages", (void **) &chat_permissions->can_send_messages },
			{ "can_send_media_messages", (void **) &chat_permissions->can_send_media_messages },
			{ "can_send_polls", (void **) &chat_permissions->can_send_polls },
			{ "can_add_web_page_previews", (void **) &chat_permissions->can_add_web_page_previews },
			{ "can_change_info", (void **) &chat_permissions->can_change_info },
			{ "can_invite_users", (void **) &chat_permissions->can_invite_users },
			{ "can_pin_messages", (void **) &chat_permissions->can_pin_messages }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_chat_photo ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_template_t chat_photo* = ( tebot_chat_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "small_file_id", (void **) &chat_photo->small_file_id },
			{ "small_file_unique_id", (void **) &chat_photo->small_file_unique_id },
			{ "big_file_id", (void **) &chat_photo->big_file_id },
			{ "big_file_unique_id", (void **) &chat_photo->big_file_unique_id }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_chat ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_chat_t chat* = ( tebot_chat_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "id", (void **) &chat->id },
			{ "type", (void **) &chat->type },
			{ "title", (void **) &chat->title },
			{ "username", (void **) &chat->username },
			{ "first_name", (void **) &chat->first_name },
			{ "last_name", (void **) &chat->last_name },
			{ "photo", (void **) &chat->photo, sizeof ( tebot_chat_photo_t ), handler_chat_photo },
			{ "bio", (void **) &chat->bio },
			{ "description", (void **) &chat->description },
			{ "invite_link", (void **) &chat->invite_link },
			{ "pinned_message", (void **) &chat->pinned_message, sizeof ( tebot_pinned_message_t ), handle_pinned_message },
			{ "permissions", (void **) &chat->permissions, sizeof ( tebot_chat_permissions_t ), handle_chat_permissions },
			{ "slow_mode_delay", (void **) &chat->slow_mode_delay },
			{ "message_auto_delete_time", (void **) &chat->message_auto_delete_time },
			{ "sticker_set_name", (void **) &chat->sticker_set_name },
			{ "can_set_sticker_set", (void **) &chat->can_set_sticker_set },
			{ "linked_chat_id", (void **) &chat->linked_chat_id },
			{ "location", (void **) &chat->location, sizeof ( tebot_chat_location_t ), handle_chat_location }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_user ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_user_t user* = ( tebot_user_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "id", (void **) &user->id },
			{ "is_bot", (void **) &user->is_bot },
			{ "first_name", (void **) &user->first_name },
			{ "last_name", (void **) &user->last_name },
			{ "username", (void **) &user->username },
			{ "language_code", (void **) &user->language_code },
			{ "can_join_groups", (void **) &user->can_join_groups },
			{ "can_read_all_group_messages", (void **) &user->can_read_all_group_messages },
			{ "supports_inline_queries", (void **) &user->supports_inline_queries }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_message ( tebot_handler_t *h, void *data, json_object *obj ) {
	tebot_message_t *msg = ( tebot_message_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {

		struct data_of_types dot[] = {
			{ "message_id", (void **) &msg->message_id },
			{ "from", (void **) &msg->from, sizeof ( tebot_user_t ), handler_user },
			{ "sender_chat", (void **) &msg->sender_chat, sizeof ( tebot_sender_chat_t ), handler_sender_chat },
			{ "date", (void **) &msg->date },
			{ "chat", (void **) &msg->chat, sizeof ( tebot_chat_t ), handler_chat },
			{ "forward_from", (void **) &msg->forward_from, sizeof ( tebot_user_t ), handler_user },
			{ "forward_from_chat", (void **) &msg->forward_from_chat, sizeof ( tebot_chat_t ), handler_chat },
			{ "forward_from_message_id", (void **) &msg->forward_from_message_id },
			{ "forward_signature", (void **) &msg->forward_signature },
			{ "forward_sender_name", (void **) &msg->forward_sender_name },
			{ "forward_date", (void **) &msg->forward_date },
			{ "reply_to_message", (void **) &msg->reply_to_message, sizeof ( tebot_message_t ), handler_message },
			{ "via_bot", (void **) &msg->via_bot, sizeof ( tebot_user_t ), handler_user },
			{ "edit_date", (void **) &msg->edit_date },
			{ "media_group_id", (void **) &msg->media_group_id },
			{ "author_signature", (void **) &msg->author_signature },
			{ "text", (void **) &msg->text },
			{ "entities", (void **) &msg->entities, sizeof ( tebot_message_entity_t ), handler_message_entity },
			{ "animation", (void **) &msg->animation, sizeof ( tebot_animation_t ), handler_animation },
			{ "audio", (void **) &msg->audio, sizeof ( tebot_audio_t ), handler_audio },
			{ "document", (void **) &msg->document, sizeof ( tebot_document_t ), handler_document },
			{ "photo", (void **) &msg->photo, sizeof ( tebot_photo_t ), handler_photo },
			{ "sticker", (void **) &msg->sticker, sizeof ( tebot_sticker_t ), handler_sticker },
			{ "video", (void **) &msg->video, sizeof ( tebot_video_t ), handler_video },
			{ "video_note", (void **) &msg->video_note, sizeof ( tebot_video_note_t ), handler_video_note },
			{ "voice", (void **) &msg->voice, sizeof ( tebot_voice_t ), handler_voice },
			{ "caption", (void **) &msg->caption },
			{ "caption_entities", (void **) &msg->caption_entities, sizeof ( tebot_message_entity_t ),
				handler_message_entity },
			{ "contact", (void **) &msg->contact, sizeof ( tebot_contact_t ), handler_contact },
			{ "dice", (void **) &msg->dice, sizeof ( tebot_dice_t ), handler_dice },
			{ "game", (void **) &msg->game, sizeof ( tebot_game_t ), handler_game },
			{ "poll", (void **) &msg->poll, sizeof ( tebot_poll_t ), handler_poll },
			{ "venue", (void **) &msg->venue, sizeof ( tebot_venue_t ), handler_venue },
			{ "location", (void **) &msg->location, sizeof ( tebot_location_t ), handler_location },
			{ "new_chat_members", (void **) &msg->new_chat_members, sizeof ( tebot_user_t ), handler_user },
			{ "left_chat_member", (void **) &msg->left_chat_member, sizeof ( tebot_user_t ), handler_user },
			{ "new_chat_title", (void **) &msg->new_chat_title },
			{ "new_chat_photo", (void **) &msg->new_chat_photo, sizeof ( tebot_photo_size_t ), handler_photo_size },
			{ "delete_chat_photo", (void **) &msg->delete_chat_photo },
			{ "group_chat_created", (void **) &msg->group_chat_created },
			{ "supergroup_chat_created", (void **) &msg->supergroup_chat_created },
			{ "channel_chat_created", (void **) &msg->channel_chat_created },
			{ "message_auto_delete_timer_changed", (void **) &msg->message_auto_delete_timer_changed,
				sizeof ( tebot_message_auto_delete_timer_changed_t ),
				handler_message_auto_delete_timer_changed },
			{ "migrate_to_chat_id", (void **) &msg->migrate_to_chat_id },
			{ "migrate_from_chat_id", (void **) &msg->migrate_from_chat_id },
			{ "pinned_message", (void **) &msg->pinned_message, sizeof ( tebot_message_t ), handler_message },
			{ "invoice", (void **) &msg->invoice, sizeof ( tebot_invoice_t ), handler_invoice },
			{ "successful_payment", (void **) &msg->successful_payment, sizeof ( tebot_successful_payment ),
				handler_successful_payment },
			{ "connected_website", (void **) &msg->connected_website },
			{ "passport_data", (void **) &msg->passport_data, sizeof ( tebot_passport_data_t ), handler_passport_data },
			{ "proximity_alert_triggered", (void **) &msg->proximity_alert_triggered, sizeof ( tebot_proximity_alert_triggered_t ),
				handler_proximity_alert_triggered_t },
			{ "voice_chat_started", (void **) &msg->voice_chat_started, sizeof ( tebot_voice_chat_started_t ),
				handler_voice_chat_started },
			{ "voice_chat_ended", (void **) &msg->voice_chat_ended, sizeof ( tebot_voice_chat_ended_t ),
				handler_voice_chat_ended },
			{ "voice_chat_participants_invited", (void **) &msg->voice_chat_participants_invited",
				sizeof ( tebot_voice_chat_participants_invited_t ),
				handler_voice_chat_participants_invited },
			{ "reply_markup", (void **) &msg->reply_markup, sizeof ( tebot_inline_keyboard_markup_t ),
				handler_inline_keyboard_markup }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}



void handler_chat ( tebot_handler_t *h, void *data, json_object *obj ) {
	printf ( "handler_chat\n" );
	tebot_chat_t *chat = ( tebot_chat_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "id", (void **) &chat->id },
			{ "title", (void **) &chat->title }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

void handler_chat_member_updated ( tebot_handler_t *h, void *data, json_object *obj ) {
	printf ( "handler_chat_member_updated\n" );
	tebot_chat_member_updated_t *msg = ( tebot_chat_member_updated_t * ) data;

	json_type type = json_object_get_type ( obj );

	if ( type == json_type_object ) {
		struct data_of_types dot[] = {
			{ "chat", (void **) &msg->chat, sizeof ( tebot_chat_t ), handler_chat }
		};

		const int size = sizeof ( dot ) / sizeof ( struct data_of_types );

		for ( int i = 0; i < size; i++ ) {
			printf ( "handler_chat_member_updated: %d of %d\n", i, size );
			parse_current_object ( h, obj, dot, i );
		}

		return;
	}

	if ( type == json_type_array ) {

		return;
	}
}

tebot_result_updated_t *tebot_method_get_updates ( tebot_handler_t *h, const int limit, const int timeout, char **allowed_updates ) {
	printf ( "get_updates\n" );

	struct mimes mimes[3] = {
		{ MIMES_TYPE_PARAM, strdup ( "limit" ), strdup_printf ( "%d", limit ) },
		{ MIMES_TYPE_PARAM, strdup ( "timeout" ), strdup_printf ( "%d", timeout ) },
		{ MIMES_TYPE_ARRAY, strdup ( "allowed_updates" ), .array = allowed_updates }
	};

	tebot_result_updated_t *t = ( tebot_result_updated_t * ) calloc ( 1, sizeof ( tebot_result_updated_t ) );
	h->res = t;
	t->update = ( tebot_update_t ** ) calloc ( limit, sizeof ( tebot_update_t * ) );

	char *data = tebot_request_get ( h, "getUpdates", mimes, 3 );

	for ( int i = 0; i < limit; i++ ) {
		t->update[i] = calloc ( 1, sizeof ( tebot_update_t ) );
		if ( !t->update[i] ) {
			log_time ( LOG_LEVEL_CRITICAL, h->log_file, h->show_debug, "failed to calloc update.\n" );
			return NULL;
		}

		struct data_of_types dot[] = {
			{ "update_id", (void **) &t->update[i]->update_id },
			{ "message", (void **) &t->update[i]->message, sizeof ( tebot_message_t ), handler_message },
			{ "edited_message", (void **) &t->update[i]->edited_message, sizeof ( tebot_message_t ), handler_message },
			{ "channel_post", (void **) &t->update[i]->channel_post, sizeof ( tebot_message_t ), handler_message },
			{ "edited_channel_post", (void **) &t->update[i]->edited_channel_post, sizeof ( tebot_message_t ), handler_message },
			{ "inline_query", (void **) &t->update[i]->inline_query, sizeof ( tebot_inline_query_t ), handler_inline_query },
			{ "chosen_inline_result", (void **) &t->update[i]->chosen_inline_result, sizeof ( tebot_chosen_inline_result_t ),
				handler_chosen_inline_result },
			{ "callback_query", (void **) &t->update[i]->callback_query, sizeof ( tebot_callback_query_t ), 
				handler_callback_query },
			{ "shipping_query", (void **) &t->update[i]->shipping_query, sizeof ( tebot_shipping_query_t ), 
				handler_shipping_query_t },
			{ "pre_checkout_query", (void **) &t->update[i]->pre_checkout_query, sizeof ( tebot_pre_checkout_query_t ),
				handler_pre_checkout_query },
			{ "poll", (void **) &t->update[i]->poll, sizeof ( tebot_poll_t ), handler_poll },
			{ "poll_answer", (void **) &t->update[i]->poll_answer, sizeof ( tebot_poll_answer_t ), handler_poll_answer },
			{ "my_chat_member", (void **) &t->update[i]->my_chat_member, sizeof ( tebot_chat_member_updated_t ), 
				handler_chat_member_updated },
			{ "chat_member", (void **) &t->update[i]->chat_member, sizeof ( tebot_chat_member_updated_t ), 
				handler_chat_member_updated }
		};

		const int size = sizeof ( dot ) / sizeof ( data_of_types );

		int ret = parse_data ( h, data, dot, size, i );
		if ( ret == -1 ) {
			log_time ( LOG_LEVEL_NOTICE, h->log_file, h->show_debug, "failed to parse data: %s\n", data );
		}
		h->size_result++;
		if ( ret == 1 ) break;
	}

	return t;
}
