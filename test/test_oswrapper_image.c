/*
This program uses oswrapper_image to decode an image file,
and display some information about the decoded data.

Usage: test_oswrapper_image (image_file.ext)
If no input is provided, it will decode an embedded version of the file named
face.png in this folder.
*/

#define OSWRAPPER_IMAGE_STATIC
#define OSWRAPPER_IMAGE_IMPLEMENTATION
#include "oswrapper_image.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* Include headers for CoInit, and link with image decoding libraries */
#include <objbase.h>
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Ole32.lib")
#endif

#include <stdio.h>
#include <stdlib.h>

unsigned char face_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0x0f, 0xbe, 0x8b, 0x00, 0x00, 0x00,
    0x21, 0x49, 0x44, 0x41, 0x54, 0x08, 0x1d, 0x63, 0x20, 0x06, 0xfc, 0xc7,
    0x8b, 0x41, 0x04, 0x0e, 0x80, 0xaa, 0x00, 0x4a, 0x23, 0xb3, 0x89, 0x37,
    0x01, 0x5d, 0x00, 0x53, 0x01, 0x5e, 0x4c, 0x08, 0x00, 0x00, 0x18, 0x04,
    0x6b, 0x95, 0x57, 0x16, 0xe2, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

unsigned int face_png_len = 90;

int main(int argc, char** argv) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    HRESULT result = CoInitialize(NULL);

    if (FAILED(result)) {
        puts("CoInitialize failed!");
        return EXIT_FAILURE;
    }

#endif
    int returnVal = EXIT_FAILURE;

    if (!oswrapper_image_init()) {
        puts("Could not initialise oswrapper_image!");
        goto exit;
    }

    const char* path = argc < 2 ? NULL : argv[argc - 1];
    int width, height, channels;
    unsigned char* image_data;

    if (path == NULL) {
        path = "(internal memory)";
        image_data = oswrapper_image_load_from_memory(face_png, face_png_len, &width, &height, &channels);
    } else {
        image_data = oswrapper_image_load_from_path(path, &width, &height, &channels);
    }

    if (image_data != NULL) {
        oswrapper_image_free(image_data);
        printf("Path: %s\nWidth: %d\nHeight: %d\nChannels: %d\n", path, width, height, channels);
        returnVal = EXIT_SUCCESS;
    } else {
        puts("Could not decode image!");
    }

    if (!oswrapper_image_uninit()) {
        puts("Could not uninitialise oswrapper_image!");
        returnVal = EXIT_FAILURE;
    }

exit:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    CoUninitialize();
#endif
    return returnVal;
}

/*
BSD Zero Clause License

Copyright (c) 2023 Ned Loynd

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/
