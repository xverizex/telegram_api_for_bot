# simple telegram api bot in C.

зависимости:
* libjson-c-dev
* curl-openssl-dev

поддерживаемые функции:
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

Мне нравиться как я придумал код для этой библиотеки. Добавление новых функций из-за этого занимает мало времени. Нужно всего лишь скопировать одну функцию и поменять в ней названия в данных. Насчёт утечек памяти не уверен, но я сделал так, что память для объектов и строк выделяется только в одном месте в библиотеке. Так что здесь трудно запутаться. Есть ещё места где выделяется память, но там вроде тоже всё нормально. Освобождение памяти происходит без сложных алгоритмов. Если параметр не нужно использовать в функции, то нужно писать `0` или `NULL`, чтобы этот параметр не учитывался в отправке данных. Чтобы узнать какие есть имена полей у структуры, вы можете посмотреть заголовок у библиотеки, либо же в документации на официальной странице telegram bot api. Все поля имеют те же названия, что и в документации. если в документации прописано `Integer` или другой, но целое число, то в моей библиотеки они все подразумеваются как `long long int`. Если в документации прописано `Float`, то в моей библиотеке подразумевается `double`.

пример:
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


пример как загрузить боту файл музыки от пользователя.
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
