CC = emcc
CXX = emcc -x c++
INCLUDES = -I..
CFLAGS += -Wall -Wextra -Os -flto --closure 1 -sASYNCIFY

all:
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_IMAGE) test_oswrapper_image.c -o test_oswrapper_image.html
	$(CXX) $(INCLUDES) $(CFLAGS) $(LDFLAGS) $(LDFLAGS_IMAGE) test_oswrapper_image.c -o test_oswrapper_image_cpp.html

clean:
	rm -f test_oswrapper_image.html test_oswrapper_image.js test_oswrapper_image.wasm test_oswrapper_image_cpp.html test_oswrapper_image_cpp.js test_oswrapper_image_cpp.wasm
