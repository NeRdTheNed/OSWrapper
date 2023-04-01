#define OSWRAPPER_IMAGE_IMPLEMENTATION
#include "oswrapper_image.h"
#include <stdlib.h>

int main(int argc, char** argv) {
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
