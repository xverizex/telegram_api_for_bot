# simple telegram api bot in C.

dependency:
* libjson-c-dev
* curl-openssl-dev
* libcreqhttp - my library http

available functions:
* getMe
* getFile
* getUpdates
* sendMessage
* sendDocument
* sendAudio
* sendPhoto
* sendVideo
* sendAnimation
* sendVoice
* sendVideoNote
* sendLocation
* sendVenue
* sendPoll
* sendChatAction
* setWebhook

# How to clone?

```
git clone --recursive https://github.com/xverizex/telegram_api_for_bot
```
If parameter is not used, then write it as 0 or NULL.

We introduced example with get_updates, if you want to use webhook then see it on https://github.com/xverizex/my_telegram_bot_example:

## Example echo
```
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <tebot.h>
#include <malloc.h>
#include "header.h"


int main ( int argc, char **argv ) {
	
	tebot_handler_t *h = tebot_init ( TOKEN, TEBOT_DEBUG_NOT_SHOW, NULL );

	long long int offset = 0;

	while ( 1 ) {
		tebot_result_updated_t *t = tebot_method_get_updates ( h, offset, 20, 0, NULL );

		for ( int i = 0; i < t->size; i++ ) {

			if ( t->update[i]->message ) {
				if ( t->update[i]->message->text ) {
					printf ( "%s: %s\n", 
							t->update[i]->message->from->username,
							t->update[i]->message->text );
				}
			}


			if ( t->update[i]->update_id > 0 ) offset = t->update[i]->update_id + 1;
		}

		tebot_free_update ( h );

		sleep ( 1 );
	}

}
```


## Example that explain how to upload the music file to bot.
```
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <tebot.h>
#include <malloc.h>
#include "header.h"


int main ( int argc, char **argv ) {
	
	tebot_handler_t *h = tebot_init ( TOKEN, TEBOT_DEBUG_NOT_SHOW, NULL );

	long long int offset = 0;

	while ( 1 ) {
		tebot_result_updated_t *t = tebot_method_get_updates ( h, offset, 20, 0, NULL );

		for ( int i = 0; i < t->size; i++ ) {

			if ( t->update[i]->message ) {
				if ( t->update[i]->message->text ) {
					printf ( "%s: %s\n", 
							t->update[i]->message->from->username,
							t->update[i]->message->text );
				}

				if ( t->update[i]->message->audio ) {
					offset = tebot_method_get_file ( h, t->update[i]->message->audio->file_id,
							t->update[i]->message->audio->file_name
							);
				}
			}


			if ( t->update[i]->update_id > 0 && t->update[i]->update_id > offset ) offset = t->update[i]->update_id + 1;
		}

		tebot_free_update ( h );

		sleep ( 1 );
	}

}
```

## Example with webhook
```
#include <stdio.h>
#include <unistd.h>
#include <tebot.h>
#include <creqhttp.h>
#include "token.h"

#define IS_DOWNLOADING_FILE                0

static tebot_handler_t *h_crypto;

static void bot_crypto_handle_msg (creqhttp_epoll_event *v, tebot_result_updated_t *t) {
	if (t->update[0]->message) {
		if (t->update[0]->message->text) {
			printf ("%s: %s\n",
					t->update[0]->message->from->username,
					t->update[0]->message->text
			       );

			struct tebot_send_message_t m;
			memset (&m, 0, sizeof (struct tebot_send_message_t));

			m.chat_id = t->update[0]->message->from->id;
			m.text = t->update[0]->message->text;

			tebot_method_send_message (h_crypto, &m);
		} 
	}

	v->is_disconnect = 1;

	char *ans = "HTTP/1.1 200 OK\r\n\r\n";
	memcpy (v->data.ans_data, ans, strlen (ans) + 1);
	v->data.ans_len = strlen (ans);
	v->data.is_answer = 1;
}

static void bot_crypto_handle_document (creqhttp_epoll_event *v, tebot_result_updated_t *t) {

	v->flags[IS_DOWNLOADING_FILE] = 0;

	if (t->update[0]->message->document) {
		printf ("%s: %s\n",
				t->update[0]->message->from->username,
				t->update[0]->message->document->file_name
		       );
	}

	if (t->update[0]->message->document) {
		char *file_id = t->update[0]->message->document->file_id;
		char *outfile = t->update[0]->message->document->file_name;

		tebot_method_get_file (h_crypto, file_id, outfile);
	}

	v->is_disconnect = 1;

	char *ans = "HTTP/1.1 200 OK\r\n\r\n";
	memcpy (v->data.ans_data, ans, strlen (ans) + 1);
	v->data.ans_len = strlen (ans);
	v->data.is_answer = 1;
}

static void bot_crypto_input_handle (creqhttp_epoll_event *v) {
       	http_req *htr = creqhttp_parse_request (v->data.data, v->data.len);

	if (htr) {
		tebot_result_updated_t *t = tebot_get_data_from_webhook (h_crypto, htr->post_data);
		if (!t) {
			v->flags[IS_DOWNLOADING_FILE] = 1;
			v->is_disconnect = 0;
			return;
		}

		bot_crypto_handle_msg (v, t);

	} else if (v->flags[IS_DOWNLOADING_FILE]) {

		tebot_result_updated_t *t = tebot_get_data_from_webhook (h_crypto, v->data.data);

		bot_crypto_handle_document (v, t);
	}
}

int main (int argc, char **argv) {

	struct tebot_setup_webhook setup_bot_crypto = {
		.port = 8080,
		.is_ssl = 1,
		.msg_handle = bot_crypto_input_handle,
		.route = "https://[your site]:8443/hook",
		.cert_file = "lets_encrypt/fullchain.pem",
		.private_key_file = "lets_encrypt/privkey.pem"
	};

	h_crypto = tebot_init (TOKEN, TEBOT_DEBUG_NOT_SHOW, NULL);

	tebot_set_webhook (h_crypto, &setup_bot_crypto);

	while (1) {
		sleep (1);
	}
}
```
