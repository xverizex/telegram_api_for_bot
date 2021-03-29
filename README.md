# simple telegram bot on C.

dependies:
* libjson-c-dev
* curl-openssl-dev

supported functions:
* getMe

example:
```
#include <stdio.h>
#include "tebot.h"

char *token = "your token";

int main ( int argc, char **argv ) {

        tebot_handler_t *tebot_handler = tebot_init ( token, TEBOT_DEBUG_SHOW, NULL );

        tebot_user_t *user = tebot_method_get_me ( tebot_handler );

        printf ( "%s: %lld\n", user->username, user->id );
}
```
