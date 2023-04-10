CC = emcc
INCLUDES = -I..
CFLAGS += -Wall -Wextra -Os -flto --closure 1 -sASYNCIFY

all:
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_IMAGE) test_oswrapper_image.c -o test_oswrapper_image.html

clean:
	rm -f test_oswrapper_image.html test_oswrapper_image.js test_oswrapper_image.wasm
