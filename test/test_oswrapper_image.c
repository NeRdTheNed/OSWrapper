#define OSWRAPPER_IMAGE_IMPLEMENTATION
#include "oswrapper_image.h"
#include <stdlib.h>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    const char* path = "face.png";
    int width, height, channels;
    unsigned char* image_data = oswrapper_image_load_from_path(path, &width, &height, &channels);

    if (image_data != NULL) {
        oswrapper_image_free(image_data);
        printf("Path: %s\nWidth: %d\nHeight: %d\nChannels: %d\n", path, width, height, channels);
        return EXIT_SUCCESS;
    }

    puts("Could not decode image!");
    return EXIT_FAILURE;
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
