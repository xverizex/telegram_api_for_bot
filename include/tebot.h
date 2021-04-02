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


typedef struct tebot_chat_permissions {
	char can_send_messages;
	char can_send_media_messages;
	char can_send_polls;
	char can_send_other_messages;
	char can_add_web_page_previews;
	char can_change_info;
	char can_invite_users;
	char can_pin_messages;
} tebot_chat_permissions_t;

#if 0
typedef struct tebot_chat_location {
	char *command;
	char *description;
} tebot_chat_location_t;
#endif
typedef struct tebot_chat_location {
	tebot_location_t *location;
	char *address;
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
	tebot_chat_permissions_t *permissions;
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

typedef struct tebot_video_note {
	char *file_id;
	char *file_unique_id;
	long long int length;
	long long int duration;
	tebot_photo_size_t *thumb;
	long long int file_size;
} tebot_video_note_t;

typedef struct tebot_voice {
	char *file_id;
	char *file_unique_id;
	long long int duration;
	char *mime_type;
	long long int file_size;
} tebot_voice_t;

typedef struct tebot_contact {
	char *phone_number;
	char *first_name;
	char *last_name;
	long long int user_id;
	char *vcard;
} tebot_contact_t;

typedef struct tebot_dice {
	char *emoji;
	long long int value;
} tebot_dice_t;

typedef struct tebot_game {
	char *title;
	char *description;
	tebot_photo_size_t **photo;
	char *text;
	tebot_message_entity_t **text_entities;
	tebot_animation_t *animation;
} tebot_game_t;

typedef struct tebot_poll_option {
	char *text;
	long long int voter_count;
} tebot_poll_option_t;

typedef struct tebot_poll {
	char *id;
	char *question;
	tebot_poll_option_t **options;
	long long int total_voter_count;
	char is_closed;
	char is_anonymous;
	char *type;
	char allows_multiple_answers;
	long long int correct_option_id;
	char *explanation;
	tebot_message_entity_t **explanation_entities;
	long long int open_period;
	long long int close_date;
} tebot_poll_t;

typedef struct tebot_venue {
	tebot_location_t *location;
	char *title;
	char *address;
	char *foursquare_id;
	char *foursquare_type;
	char *google_place_id;
	char *google_place_type;
} tebot_venue_t;

typedef struct tebot_message_auto_delete_timer_changed {
	long long int message_auto_delete_time;
} tebot_message_auto_delete_timer_changed_t;

typedef struct tebot_invoice {
	char *title;
	char *description;
	char *start_parameter;
	char *currency;
	long long int total_amount;
} tebot_invoice_t;

typedef struct tebot_shipping_address {
	char *country_code;
	char *state;
	char *city;
	char *street_line1;
	char *street_line2;
	char *post_code;
} tebot_shipping_address_t;

typedef struct tebot_order_info {
	char *name;
	char *phone_number;
	char *email;
	tebot_shipping_address_t *shipping_address;
} tebot_order_info_t;

typedef struct tebot_successful_payment {
	char *currency;
	long long int total_amount;
	char *invoice_payload;
	char *shipping_option_id;
	tebot_order_info_t *order_info;
	char *telegram_payment_charge_id;
	char *provider_payment_charge_id;
} tebot_successful_payment_t;

typedef struct tebot_passport_file {
	char *file_id;
	char *file_unique_id;
	long long int file_size;
	long long int file_date;
} tebot_passport_file_t;

typedef struct tebot_encrypted_passport_element {
	char *type;
	char *data;
	char *phone_number;
	char *email;
	tebot_passport_file_t **files;
	tebot_passport_file_t *front_side;
	tebot_passport_file_t *reverse_side;
	tebot_passport_file_t *selfie;
	tebot_passport_file_t **translation;
	char *hash;

} tebot_encrypted_passport_element_t;

typedef struct tebot_encrypted_credentials {
	char *data;
	char *hash;
	char *secret;
} tebot_encrypted_credentials_t;

typedef struct tebot_passport_data {
	tebot_encrypted_passport_element_t **data;
	tebot_encrypted_credentials_t *credentials;
} tebot_passport_data_t;

typedef struct tebot_proximity_alert_triggered {
	tebot_user_t *traveler;
	tebot_user_t *watcher;
	long long int distance;
} tebot_proximity_alert_triggered_t;

typedef struct tebot_voice_chat_started {
} tebot_voice_chat_started_t;

typedef struct tebot_voice_chat_ended {
	long long int duration;
} tebot_voice_chat_ended_t;

typedef struct tebot_voice_chat_participants_invited {
	tebot_user_t **users;
} tebot_voice_chat_participants_invited_t;

typedef struct tebot_login_url {
	char *url;
	char *forward_text;
	char *bot_username;
	char request_write_access;
} tebot_login_url_t;

typedef struct tebot_callback_game {
} tebot_callback_game_t;

typedef struct tebot_inline_keyboard_button {
	char *text;
	char *url;
	tebot_login_url_t *login_url;
	char *callback_data;
	char *switch_inline_query;
	char *switch_inline_query_current_chat;
	tebot_callback_game_t *callback_game;
	char pay;
} tebot_inline_keyboard_button_t;

typedef struct tebot_inline_keyboard_markup {
	tebot_inline_keyboard_button_t **inline_keyboard;
} tebot_inline_keyboard_markup_t;

typedef struct tebot_keyboard_button_poll_type {
	char *type;
} tebot_keyboard_button_poll_type_t;

typedef struct tebot_keyboard_button {
	char *text;
	char request_contact;
	char request_location;
	tebot_keyboard_button_poll_type_t *request_poll;
} tebot_keyboard_button_t;

typedef struct tebot_reply_keyboard_markup {
	tebot_keyboard_button_t **keyboard;
	char resize_keyboard;
	char one_time_keyboard;
	char selective;
} tebot_reply_keyboard_markup_t;

typedef struct tebot_reply_keyboard_remove {
	char remove_keyboard;
	char selective;
} tebot_reply_keyboard_remove_t;

typedef struct tebot_force_reply {
	char force_reply;
	char selective;
} tebot_force_reply_t;

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
	long long int size_entities;
	tebot_animation_t *animation;
	tebot_audio_t *audio;
	tebot_document_t *document;
	tebot_photo_size_t **photo;
	tebot_sticker_t *sticker;
	tebot_video_t *video;
	tebot_video_note_t *video_note;
	tebot_voice_t *voice;
	char *caption;
	tebot_message_entity_t **caption_entities;
	tebot_contact_t *contact;
	tebot_dice_t *dice;
	tebot_game_t *game;
	tebot_poll_t *poll;
	tebot_venue_t *venue;
	tebot_location_t *location;
	tebot_user_t **new_chat_members;
	tebot_user_t *left_chat_member;
	char *new_chat_title;
	tebot_photo_size_t *new_chat_photo;
	char delete_chat_photo;
	char group_chat_created;
	char supergroup_chat_created;
	char channel_chat_created;
	tebot_message_auto_delete_timer_changed_t *message_auto_delete_timer_changed;
	long long int migrate_to_chat_id;
	long long int migrate_from_chat_id;
	tebot_message_t *pinned_message;
	tebot_invoice_t *invoice;
	tebot_successful_payment_t *successful_payment;
	char *connected_website;
	tebot_passport_data_t *passport_data;
	tebot_proximity_alert_triggered_t *proximity_alert_triggered;
	tebot_voice_chat_started_t *voice_chat_started;
	tebot_voice_chat_ended_t *voice_chat_ended;
	tebot_voice_chat_participants_invited_t *voice_chat_participants_invited;
	tebot_inline_keyboard_markup_t *reply_markup;

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

typedef struct tebot_shipping_query {
	char *id;
	tebot_user_t *from;
	char *invoice_payload;
	tebot_shipping_address_t *shipping_address;
} tebot_shipping_query_t;

typedef struct tebot_pre_checkout_query {
	char *id;
	tebot_user_t *from;
	char *currency;
	long long int total_amount;
	char *invoice_payload;
	char *shipping_option_id;
	tebot_order_info_t *order_info;
} tebot_pre_checkout_query_t;

typedef struct tebot_poll_answer {
	char *poll_id;
	tebot_user_t *user;
	long long int **option_ids;
} tebot_poll_answer_t;

typedef struct tebot_chat_member {
	tebot_user_t *user;
	char *status;
	char *custom_title;
	char is_anonymous;
	char can_be_edited;
	char can_manage_chat;
	char can_post_messages;
	char can_edit_messages;
	char can_delete_messages;
	char can_manage_voice_chats;
	char can_restrict_members;
	char can_promote_members;
	char can_change_info;
	char can_invite_users;
	char can_pin_messages;
	char is_member;
	char can_send_messages;
	char can_send_media_messages;
	char can_send_polls;
	char can_send_other_messages;
	char can_add_web_page_previews;
	long long int until_date;
} tebot_chat_member_t;

typedef struct tebot_chat_invite_link {
	char *invite_link;
	tebot_user_t *creator;
	char is_primary;
	char is_revoked;
	long long int expire_date;
	long long int member_limit;
} tebot_chat_invite_link_t;

typedef struct tebot_chat_member_updated {
	tebot_chat_t *chat;
	tebot_user_t *from;
	long long int date;
	tebot_chat_member_t *old_chat_member;
	tebot_chat_member_t *new_chat_member;
	tebot_chat_invite_link_t *invite_link;
} tebot_chat_member_updated_t;

typedef struct tebot_update {
	long long int update_id;
	tebot_message_t *message;
	tebot_message_t *edited_message;
	tebot_message_t *channel_post;
	tebot_message_t *edited_channel_post;
	tebot_inline_query_t *inline_query;
	tebot_chosen_inline_result_t *chosen_inline_result;
	tebot_callback_query_t *callback_query;
	tebot_shipping_query_t *shipping_query;
	tebot_pre_checkout_query_t *pre_checkout_query;
	tebot_poll_t *poll;
	tebot_poll_answer_t *poll_answer;
	tebot_chat_member_updated_t *my_chat_member;
	tebot_chat_member_updated_t *chat_member;
} tebot_update_t;

typedef struct tebot_result_updated {
	tebot_update_t **update;
	int size;
} tebot_result_updated_t;

typedef struct tebot_handler {
	CURL *curl;
	int show_debug;
	const char *log_file;
	char *token;
	char *url_get;
	char *current_buf;
	long long int offset;
	tebot_result_updated_t *res;
	void **for_free;
	int size_for_free;
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

typedef enum tebot_reply_markup {
	INLINE_KEYBOARD_MARKUP,
	REPLY_KEYBOARD_MARKUP,
	REPLY_KEYBOARD_REMOVE,
	FORCE_REPLY
} tebot_reply_markup_enum;

tebot_handler_t *tebot_init ( const char *token, const tebot_show_debug_enum show_debug, const char *log_file );
tebot_user_t *tebot_method_get_me ( tebot_handler_t *handler );
tebot_result_updated_t *tebot_method_get_updates ( tebot_handler_t *handler, const long long int offset,
		const int limit, const int timeout, char **allowed_updates );

void tebot_method_send_message ( tebot_handler_t *h, long long int chat_id, 
		char *text,
		char *parse_mode,
		char disable_web_page_preview,
		char disable_notification,
		long long int reply_to_message_id,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout
		);

void tebot_method_send_document ( tebot_handler_t *h, long long int chat_id, 
		char *document,
		char *thumb,
		char *caption,
		char *parse_mode,
		char disable_content_type_detection,
		char disable_notification,
		const long long int reply_to_message_id,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout
		);

void tebot_method_send_audio ( tebot_handler_t *h, long long int chat_id, 
		char *audio,
		char *thumb,
		long long int duration,
		char *performer,
		char *title,
		char *caption,
		char *parse_mode,
		char disable_content_type_detection,
		char disable_notification,
		const long long int reply_to_message_id,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout
		);

void tebot_method_send_photo ( tebot_handler_t *h, long long int chat_id, 
		char *photo,
		char *caption,
		char *parse_mode,
		char disable_notification,
		const long long int reply_to_message_id,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout );

void tebot_method_send_video ( tebot_handler_t *h, long long int chat_id, 
		char *video,
		char *caption,
		char *parse_mode,
		char *thumb,
		long long int duration,
		long long int width,
		long long int height,
		long long int reply_to_message_id,
		char disable_notification,
		char supports_streaming,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout );

void tebot_method_send_animation ( tebot_handler_t *h, long long int chat_id, 
		char *animation,
		char *caption,
		char *parse_mode,
		char *thumb,
		long long int duration,
		long long int width,
		long long int height,
		long long int reply_to_message_id,
		char disable_notification,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout );

void tebot_method_send_voice ( tebot_handler_t *h, long long int chat_id, 
		char *voice,
		long long int duration,
		char *caption,
		char *parse_mode,
		char disable_notification,
		const long long int reply_to_message_id,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout
		);

void tebot_method_send_video_note ( tebot_handler_t *h, long long int chat_id, 
		char *video_note,
		char *caption,
		char *parse_mode,
		char *thumb,
		long long int duration,
		long long int length,
		char disable_notification,
		const long long int reply_to_message_id,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_of_reply_markup,
		int layout[],
		int size_layout
		);

void tebot_method_forwardMessage ( tebot_handler_t *h,
		long long int chat_id, 
		long long int from_chat_id, 
		char disable_notification, 
		long long int message_id );

void tebot_method_copy_message ( tebot_handler_t *h,
		long long int chat_id,
		long long int from_chat_id,
		long long int message_id,
		char *caption,
		char *parse_mode,
		char disable_notification,
		long long int reply_to_message_id,
		char allow_sending_without_reply,
		void *reply_markup,
		int type_reply_markup,
		int layout[],
		int size_layout
		);

void tebot_free_update ( tebot_handler_t *h );

tebot_inline_keyboard_markup_t *tebot_init_inline_keyboard_markup ( const int size );
tebot_reply_keyboard_markup_t *tebot_init_reply_keyboard_markup ( const int size );
tebot_message_entity_t **tebot_init_message_entity ( const int size );

#ifdef __cplusplus
}
#endif        // __cplusplus

#endif
