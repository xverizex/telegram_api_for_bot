# simple telegram bot on C.

dependies:
* libjson-c-dev
* curl-openssl-dev

supported functions:
* getMe
* getUpdates
* sendMessage

Код не смотрите, плакать будете. )

Я думаю что всё усложнил. Теперь для каждого update для существующих данных выделяется память, только для тех, которые есть во входящих данных. Не знаю правильный ли это подход. Для long, double и bool не выделяется память, для строк и объектов выделяется. и так при каждом запросе.

example:
```
#include <stdio.h>
#include <unistd.h>
#include <tebot.h>

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

		tebot_free_update ( tebot_handler );
		
		sleep ( 1 );
	}
}
```

вот как отправить кнопки.
```
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <tebot.h>

char *token = "your token";

int main ( int argc, char **argv ) {
	
	tebot_handler_t *h = tebot_init ( token, TEBOT_DEBUG_NOT_SHOW, NULL );

	long long int offset = 0;

	tebot_inline_keyboard_markup_t *m = tebot_init_inline_keyboard_markup ( 3 );
	m->inline_keyboard[0]->text = strdup ( "hello" );
	m->inline_keyboard[0]->callback_data = strdup ( "button_netto" );
	m->inline_keyboard[1]->text = strdup ( "netto" );
	m->inline_keyboard[1]->callback_data = strdup ( "button_hello" );
	m->inline_keyboard[2]->text = strdup ( "hillo" );
	m->inline_keyboard[2]->callback_data = strdup ( "button_hillo" );

	while ( 1 ) {
		tebot_result_updated_t *t = tebot_method_get_updates ( h, offset, 20, 0, NULL );

		for ( int i = 0; i < t->size; i++ ) {

			if ( t->update[i]->message ) {
				if ( t->update[i]->message->text ) {
					if ( !strncmp ( t->update[i]->message->text, "/start", 7 ) ) {
						tebot_method_send_message ( h,
								t->update[i]->message->from->id,
								"выберите кнопку",
								NULL,
								NULL,
								-1,
								-1,
								-1,
								-1,
								m,
								INLINE_KEYBOARD_MARKUP
								);

					}
				}
			}


			if ( t->update[i]->update_id > 0 ) offset = t->update[i]->update_id + 1;
		}

		tebot_free_update ( h );

		sleep ( 1 );
	}

}
```
