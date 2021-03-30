# simple telegram bot on C.

dependies:
* libjson-c-dev
* curl-openssl-dev

supported functions:
* getMe
* getUpdates
* sendMessage

Код не смотрите, плакать будете. )

Я думаю что всё усложнил. Теперь для каждого update для существующих данных выделяется память, только для тех, которые есть во входящих данных. Не знаю правильный ли это подход. Для long, double и bool не выделяется память, для строк и объектов выделяется. и так при каждом запросе. в первом примере создаются inline кнопки, в layout указываются сколько кнопок будет распологаться в каждой строке.

example:
```
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <tebot.h>

char *token = "";

int main ( int argc, char **argv ) {
	
	tebot_handler_t *h = tebot_init ( token, TEBOT_DEBUG_NOT_SHOW, NULL );

	long long int offset = 0;

	tebot_inline_keyboard_markup_t *m = tebot_init_inline_keyboard_markup ( 8 );
	m->inline_keyboard[0]->text = strdup ( "hello" );
	m->inline_keyboard[0]->callback_data = strdup ( "button_hello" );
	m->inline_keyboard[1]->text = strdup ( "netto" );
	m->inline_keyboard[1]->callback_data = strdup ( "button_netto" );
	m->inline_keyboard[2]->text = strdup ( "hello" );
	m->inline_keyboard[2]->callback_data = strdup ( "button_hello" );
	m->inline_keyboard[3]->text = strdup ( "netto" );
	m->inline_keyboard[3]->callback_data = strdup ( "button_netto" );
	m->inline_keyboard[4]->text = strdup ( "hello" );
	m->inline_keyboard[4]->callback_data = strdup ( "button_hello" );
	m->inline_keyboard[5]->text = strdup ( "netto" );
	m->inline_keyboard[5]->callback_data = strdup ( "button_netto" );
	m->inline_keyboard[6]->text = strdup ( "netto" );
	m->inline_keyboard[6]->callback_data = strdup ( "button_netto" );
	m->inline_keyboard[7]->text = strdup ( "netto" );
	m->inline_keyboard[7]->callback_data = strdup ( "button_netto" );

	int layout[] = { 3, 2, 3 };

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
								INLINE_KEYBOARD_MARKUP,
								layout,
								3	
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

