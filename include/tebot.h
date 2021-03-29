#ifndef __TEBOT__
#define __TEBOT__

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif        // __cplusplus

#define URL_API                       "https://api.telegram.org"

typedef struct tebot_user {
	long long int id;
	char is_bot;
	char *first_name;
	char *last_name;
	char *username;
	char *language_code;
	char can_join_groups;
	char can_read_all_group_messages;
	char supports_inline_queries;
} tebot_user_t;

typedef struct tebot_handler {
	CURL *curl;
	int show_debug;
	const char *log_file;
	char *token;
	char *url_get;
	char *current_buf;
	int offset;

	/* types */
	tebot_user_t user;

} tebot_handler_t;

typedef enum tebot_show_debug {
	TEBOT_DEBUG_NOT_SHOW = 0,
	TEBOT_DEBUG_SHOW = 1
} tebot_show_debug_enum;

typedef enum tebot_log_level {
	LOG_LEVEL_CRITICAL,
	LOG_LEVEL_NOTICE,
	LOG_LEVEL_BAD_REQUEST
} tebot_log_level_enum;

tebot_handler_t *tebot_init ( const char *token, const tebot_show_debug_enum show_debug, const char *log_file );
tebot_user_t *tebot_method_get_me ( tebot_handler_t *handler );

#ifdef __cplusplus
}
#endif        // __cplusplus

#endif
