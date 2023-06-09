/*
This program uses oswrapper_audio to decode an audio file,
and write the raw PCM data to a file with the same name with the extension .raw
appended to it.

Usage: test_oswrapper_audio (audio_file.ext)
If no input is provided, it will decode the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_audio.c
*/

#define OSWRAPPER_AUDIO_STATIC
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <objbase.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Ole32.lib")
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef HINT_OUTPUT_FORMAT
#define SAMPLE_RATE 44100
#define CHANNEL_COUNT 2
#define BITS_PER_CHANNEL 16
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER
#define ENDIANNESS_TYPE OSWRAPPER_AUDIO_ENDIANNESS_LITTLE
#else
#define SAMPLE_RATE 0
#define CHANNEL_COUNT 0
#define BITS_PER_CHANNEL 0
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_NOT_SET
#define ENDIANNESS_TYPE OSWRAPPER_AUDIO_ENDIANNESS_USE_SYSTEM_DEFAULT
#endif

#define TEST_PROGRAM_BUFFER_SIZE 0x50

/* Decodes a given audio file to raw PCM data */
int main(int argc, char** argv) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    HRESULT result = CoInitialize(NULL);

    if (FAILED(result)) {
        puts("CoInitialize failed!");
        return EXIT_FAILURE;
    }

#endif
    int returnVal = EXIT_FAILURE;
    FILE* output_file = NULL;
    OSWrapper_audio_spec* audio_spec = NULL;
    char* output_path = NULL;
    short* buffer = NULL;
    const char* path = argc < 2 ? "noise.wav" : argv[argc - 1];
    size_t input_string_length = strlen(path);

    if (!oswrapper_audio_init()) {
        puts("Could not initialise oswrapper_audio!");
        goto exit;
    }

    audio_spec = (OSWrapper_audio_spec*) calloc(1, sizeof(OSWrapper_audio_spec));

    if (audio_spec == NULL) {
        puts("calloc failed for OSWrapper_audio_spec!");
        goto exit;
    }

    output_path = (char*) malloc(input_string_length + sizeof(".raw"));

    if (output_path == NULL) {
        puts("malloc failed for output path!");
        goto exit;
    }

    memcpy(output_path, path, input_string_length);
    memcpy(output_path + input_string_length, ".raw", sizeof(".raw"));
    output_file = fopen(output_path, "rb");

    if (output_file != NULL) {
        fclose(output_file);
        output_file = NULL;
        printf("Output file %s already exists!\n", output_path);
        goto exit;
    }

    output_file = fopen(output_path, "wb");

    if (output_file == NULL) {
        printf("Output file %s could not be opened for writing!\n", output_path);
        goto exit;
    }

    audio_spec->sample_rate = SAMPLE_RATE;
    audio_spec->channel_count = CHANNEL_COUNT;
    audio_spec->bits_per_channel = BITS_PER_CHANNEL;
    audio_spec->audio_type = AUDIO_FORMAT;
    audio_spec->endianness_type = ENDIANNESS_TYPE;

    if (oswrapper_audio_load_from_path(path, audio_spec)) {
        printf("Path: %s\nOutput path: %s\nSample rate: %lu\nChannels: %d\nBit depth: %d\n", path, output_path, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel);

        if (audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
            puts("Output format: floating point PCM\n");
        } else {
            puts("Output format: integer PCM\n");
        }

        if (audio_spec->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG) {
            puts("Big-endian\n");
        } else {
            puts("Little-endian\n");
        }

        size_t frame_size = (audio_spec->bits_per_channel / 8) * (audio_spec->channel_count);
        buffer = (short*) calloc(TEST_PROGRAM_BUFFER_SIZE, frame_size);
        size_t frames_done = 0;

        if (buffer == NULL) {
            puts("calloc failed for audio decoding buffer!");
            goto audio_cleanup;
        }

        while (1) {
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);

            if (this_iter == 0) {
                break;
            }

            fwrite(buffer, frame_size, this_iter, output_file);
            frames_done += this_iter;
        }

        fclose(output_file);
        output_file = NULL;
        printf("Decoded %zu frames of audio, with frame size %zu\n", frames_done, frame_size);
        returnVal = EXIT_SUCCESS;
audio_cleanup:

        if (!oswrapper_audio_free_context(audio_spec)) {
            puts("Could not free audio context!");
        }
    } else {
        puts("Could not decode audio!");
    }

    if (!oswrapper_audio_uninit()) {
        puts("Could not uninitialise oswrapper_audio!");
        returnVal = EXIT_FAILURE;
    }

exit:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    CoUninitialize();
#endif

    if (output_file != NULL) {
        fclose(output_file);
        output_file = NULL;
    }

    if (audio_spec != NULL) {
        free(audio_spec);
        audio_spec = NULL;
    }

    if (output_path != NULL) {
        free(output_path);
        output_path = NULL;
    }

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

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
