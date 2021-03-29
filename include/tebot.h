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

typedef struct tebot_location {
	double longitude;
	double latitude;
	double horizontal_accuracy;
	long live_period;
	long heading;
	long proximity_alert_radius;
} tebot_location_t;

typedef struct tebot_inline_query {
	char *id;
	tebot_user_t *from;
	tebot_location_t *location;
	char *query;
	char *offset;
} tebot_inline_query_t;

typedef struct tebot_chosen_inline_result {
	char *result_id;
	tebot_user_t *from;
	tebot_location_t *location;
	char *inline_message_id;
	char *query;
} tebot_chosen_inline_result_t;

typedef struct tebot_chat_photo {
	char *small_file_id;
	char *small_file_unique_id;
	char *big_file_id;
	char *big_file_unique_id;
} tebot_chat_photo_t;


typedef struct tebot_chat_permission {
	char can_send_messages;
	char can_send_media_messages;
	char can_send_polls;
	char can_send_other_messages;
	char can_add_web_page_previews;
	char can_change_info;
	char can_invite_users;
	char can_pin_messages;
} tebot_chat_permission_t;

typedef struct tebot_chat_location {
	char *command;
	char *description;
} tebot_chat_location_t;

typedef struct tebot_message tebot_message_t;

typedef struct tebot_chat {
	long long int id;
	char *type;
	char *title;
	char *username;
	char *first_name;
	char *last_name;
	tebot_chat_photo_t *photo;
	char *bio;
	char *description;
	char *invite_link;
	tebot_message_t *pinned_message;
	tebot_chat_permission_t *permissions;
	long long int slow_mode_delay;
	long long int message_auto_delete_time;
	char *sticker_set_name;
	char can_set_sticker_set;
	long long int linked_chat_id;
	tebot_chat_location_t *location;
} tebot_chat_t;

typedef struct tebot_message_entity {
	char *type;
	long long int offset;
	long long int length;
	char *url;
	tebot_user_t *user;
	char *language;
} tebot_message_entity_t;

typedef struct tebot_photo_size {
	char *file_id;
	char *file_unique_id;
	long long int width;
	long long int height;
	long long int file_size;
} tebot_photo_size_t;

typedef struct tebot_animation {
	char *file_id;
	char *file_unique_id;
	long long int width;
	long long int height;
	long long int duration;
	tebot_photo_size_t *thumb;
	char *file_name;
	char *mime_type;
	long long int file_size;
} tebot_animation_t;

typedef struct tebot_audio {
	char *file_id;
	char *file_unique_id;
	long long int duration;
	char *performer;
	char *title;
	char *file_name;
	char *mime_type;
	long long int file_size;
	tebot_photo_size_t *thumb;
} tebot_audio_t;

typedef struct tebot_document {
	char *file_id;
	char *file_unique_id;
	tebot_photo_size_t *thumb;
	char *file_name;
	char *mime_type;
	long long int file_size;
} tebot_document_t;

typedef struct tebot_mask_position {
	char *point;
	double x_shift;
	double y_shift;
	double scale;
} tebot_mask_position_t;

typedef struct tebot_sticker {
	char *file_id;
	char *file_unique_id;
	long long int width;
	long long int height;
	char is_animated;
	tebot_photo_size_t *thumb;
	char *emoji;
	char *set_name;
	tebot_mask_position_t *mask_position;
	long long int file_size;
} tebot_sticker_t;

typedef struct tebot_video {
	char *file_id;
	char *file_unique_id;
	long long int width;
	long long int height;
	long long int duration;
	tebot_photo_size_t *thumb;
	char *file_name;
	char *mime_type;
	long long int file_size;
} tebot_video_t;

typedef struct tebot_message {
	long long int message_id;
	tebot_user_t *from;
	tebot_chat_t *sender_chat;
	long long int date;
	tebot_chat_t *chat;
	tebot_user_t *forward_from;
	tebot_chat_t *forward_from_chat;
	long long int forward_from_message_id;
	char *forward_signature;
	char *forward_sender_name;
	long long int forward_date;
	tebot_message_t *reply_to_message;
	tebot_user_t *via_bot;
	long long int edit_date;
	char *media_group_id;
	char *author_signature;
	char *text;
	tebot_message_entity_t **entities;
	tebot_animation_t *animation;
	tebot_audio_t *audio;
	tebot_document_t *document;
	tebot_photo_size_t **photo;
	tebot_sticker_t *sticker;
	tebot_video_t *video;

} tebot_message_t;

typedef struct tebot_callback_query {
	char *id;
	tebot_user_t *from;
	tebot_message_t *message;
	char *inline_message_id;
	char *chat_instance;
	char *data;
	char *game_short_name;
} tebot_callback_query_t;

typedef struct tebot_update {
	int update_id;
	tebot_message_t *message;
	tebot_message_t *edited_message;
	tebot_message_t *channel_post;
	tebot_message_t *edited_channel_post;
	tebot_inline_query_t *inline_query;
	tebot_chosen_inline_result_t *chosen_inline_result;

} tebot_update_t;

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
