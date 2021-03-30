# simple telegram bot on C.

dependies:
* libjson-c-dev
* curl-openssl-dev

supported functions:
* getMe
* getUpdates
* sendMessage

пока не реализовано освобождение памяти. видимо я всё усложнил и теперь надо думать как освобождать этот ужас. код не смотрите, плакать будете. )

example:
```
#include <stdio.h>
#include <unistd.h>
#include "tebot.h"

char *token = "";

int main ( int argc, char **argv ) {

	tebot_handler_t *tebot_handler = tebot_init ( token, TEBOT_DEBUG_SHOW, NULL );

	tebot_user_t *user = tebot_method_get_me ( tebot_handler );


	long long int offset = 0;
	while ( 1 ) {
		tebot_result_updated_t *upd = tebot_method_get_updates ( tebot_handler, offset, 20, 0, NULL );
		for ( int i = 0; i < upd->size; i++ ) {

			if ( upd->update[i]->message ) {
				printf ( "%s from %s\n",
						upd->update[i]->message->text,
						upd->update[i]->message->from->username
				       );

				tebot_method_send_message ( tebot_handler,
						upd->update[i]->message->from->id,
						upd->update[i]->message->text,
						NULL,
						NULL,
						1,
						0,
						-1,
						1,
						NULL,
						-1
						);
			}

			if ( upd->update[i]->update_id > 0 ) offset = upd->update[i]->update_id + 1;
		}
		
		sleep ( 1 );
	}
}
```
