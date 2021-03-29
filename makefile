LIBS=`pkg-config json-c,libcurl --libs --cflags`
all:
	gcc src/tebot.c -Iinclude/ -fPIC -shared $(LIBS) -o libtebot.so
clean:
	rm libtebot.so
