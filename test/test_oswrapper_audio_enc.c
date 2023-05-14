/*
This program uses oswrapper_audio to decode an audio file,
and encodes the decoded PCM data to a variety of file types using oswrapper_audio_enc.

Usage: test_oswrapper_audio_enc (audio_file.ext) (optional wanted file format)
If no input is provided, it will decode and encode the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_audio_enc.c
*/

#define OSWRAPPER_AUDIO_STATIC
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#define OSWRAPPER_AUDIO_ENC_STATIC
#define OSWRAPPER_AUDIO_ENC_IMPLEMENTATION
#include "oswrapper_audio_enc.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <objbase.h>
#pragma comment(lib, "mf.lib")
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

static OSWrapper_audio_enc_output_type demo_get_enum_for_str(const char* type) {
    if (strcmp(type, "alac") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC;
    } else if (strcmp(type, "flac") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC;
    } else if (strcmp(type, "wav") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV;
    } else if (strcmp(type, "snd") == 0 || strcmp(type, "au") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND;
    } else if (strcmp(type, "m4a") == 0 || strcmp(type, "aac") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC;
    } else if (strcmp(type, "aac-he") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC_HE;
    } else if (strcmp(type, "mp2") == 0 || strcmp(type, "mpg") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_MPEG;
    } else if (strcmp(type, "mp3") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_MP3;
    } else if (strcmp(type, "wma-speech") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_SPEECH;
    } /* else if (strcmp(type, "wma-lossless") == 0) {

        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_LOSSLESS;
    } */ else if (strcmp(type, "wma-v9") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_V9;
    } else if (strcmp(type, "wma-v8") == 0 || strcmp(type, "wma") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_V8;
    } else if (strcmp(type, "opus") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_OPUS;
    } else {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV;
    }
}

static const char* demo_get_ext_for_enum(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND:
        return ".snd";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC_HE:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC:
        return ".m4a";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_MPEG:
        return ".mpg";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_MP3:
        return ".mp3";

    /* case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_LOSSLESS: */
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_V8:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_V9:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WMA_SPEECH:
        return ".wma";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_OPUS:
        return ".opus";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC:
        return ".flac";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV:
    default:
        return ".wav";
    }
}

/* Decodes and re-encodes a given audio file to a variety of file types */
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
    OSWrapper_audio_enc_spec* audio_enc_spec = NULL;
    char* output_path = NULL;
    short* buffer = NULL;
    const char* path;
    const char* type;
    const char* ext = NULL;

    if (argc > 2) {
        path = argv[argc - 2];
        type = argv[argc - 1];
    } else {
        path = argc < 2 ? "noise.wav" : argv[argc - 1];
        type = "m4a";
    }

    OSWrapper_audio_enc_output_type output_type = demo_get_enum_for_str(type);
    ext = demo_get_ext_for_enum(output_type);
    size_t input_string_length = strlen(path);
    size_t output_ext_length = strlen(ext) + 1;

    if (!oswrapper_audio_init()) {
        puts("Could not initialise oswrapper_audio!");
        goto exit;
    }

    if (!oswrapper_audio_enc_init()) {
        puts("Could not initialise oswrapper_audio_enc!");
        goto exit;
    }

    audio_spec = (OSWrapper_audio_spec*) calloc(1, sizeof(OSWrapper_audio_spec));

    if (audio_spec == NULL) {
        puts("calloc failed for OSWrapper_audio_spec!");
        goto exit;
    }

    audio_enc_spec = (OSWrapper_audio_enc_spec*) calloc(1, sizeof(OSWrapper_audio_enc_spec));

    if (audio_enc_spec == NULL) {
        puts("calloc failed for OSWrapper_audio_enc_spec!");
        goto exit;
    }

    output_path = (char*) malloc(input_string_length + output_ext_length);

    if (output_path == NULL) {
        puts("malloc failed for output path!");
        goto exit;
    }

    memcpy(output_path, path, input_string_length);
    memcpy(output_path + input_string_length, ext, output_ext_length);
    output_file = fopen(output_path, "rb");

    if (output_file != NULL) {
        fclose(output_file);
        output_file = NULL;
        printf("Output file %s already exists!\n", output_path);
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
        /* Input data */
        audio_enc_spec->input_data.sample_rate = audio_spec->sample_rate;
        audio_enc_spec->input_data.channel_count = audio_spec->channel_count;
        audio_enc_spec->input_data.bits_per_channel = audio_spec->bits_per_channel;
        audio_enc_spec->input_data.pcm_type = audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT ? OSWRAPPER_AUDIO_ENC_PCM_FLOAT : OSWRAPPER_AUDIO_ENC_PCM_INTEGER;
        audio_enc_spec->input_data.pcm_endianness_type = audio_spec->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG ? OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG : OSWRAPPER_AUDIO_ENC_ENDIANNESS_LITTLE;
        /* Output data */
        audio_enc_spec->output_type = output_type;

        if (!oswrapper_audio_enc_make_file_from_path(output_path, audio_enc_spec)) {
            puts("Could not encode audio!");
            goto dec_cleanup;
        }

        if (buffer == NULL) {
            puts("calloc failed for audio decoding buffer!");
            goto audio_cleanup;
        }

        while (1) {
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);

            if (this_iter == 0) {
                break;
            }

            if (!oswrapper_audio_enc_encode_samples(audio_enc_spec, buffer, this_iter)) {
                printf("Error writing frame at position %zu!\n", frames_done);
            }

            frames_done += this_iter;
        }

        printf("Decoded %zu frames of audio, with frame size %zu\n", frames_done, frame_size);
        returnVal = EXIT_SUCCESS;
audio_cleanup:

        if (!oswrapper_audio_enc_finalise_file_context(audio_enc_spec)) {
            puts("Could not free audio encoding context!");
        }

dec_cleanup:

        if (!oswrapper_audio_free_context(audio_spec)) {
            puts("Could not free audio decoding context!");
        }
    } else {
        puts("Could not decode audio!");
    }

    if (!oswrapper_audio_enc_uninit()) {
        puts("Could not uninitialise oswrapper_audio_enc!");
        returnVal = EXIT_FAILURE;
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

    if (audio_enc_spec != NULL) {
        free(audio_enc_spec);
        audio_enc_spec = NULL;
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
