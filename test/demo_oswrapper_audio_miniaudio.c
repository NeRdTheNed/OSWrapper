/*
This program uses oswrapper_audio to decode and play an audio file twice,
using miniaudio for sound output.

Usage: demo_oswrapper_audio_miniaudio (audio_file.ext)
If no input is provided, it will play the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/demo_oswrapper_audio_miniaudio.c
*/

#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <objbase.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Ole32.lib")
#endif

#include <stdlib.h>

#ifdef HINT_OUTPUT_FORMAT
#define SAMPLE_RATE 44100
#define CHANNEL_COUNT 2
#define BITS_PER_CHANNEL 16
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER
#else
#define SAMPLE_RATE 0
#define CHANNEL_COUNT 0
#define BITS_PER_CHANNEL 0
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_NOT_SET
#endif

static ma_event stop_audio_cond;

#define FAIL_WITH_MESSAGE_ON_COND(cond, message) if ((cond)) { puts(message); return EXIT_FAILURE; }
#define EXIT_WITH_MESSAGE_ON_COND(cond, message) if ((cond)) { puts(message); exit(EXIT_FAILURE); }

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    OSWrapper_audio_spec* audio_spec = (OSWrapper_audio_spec*)pDevice->pUserData;

    if (audio_spec == NULL) {
        EXIT_WITH_MESSAGE_ON_COND((ma_event_signal(&stop_audio_cond) != MA_SUCCESS), "Could not send signal to main thread!");
        return;
    }

    if (oswrapper_audio_get_samples(audio_spec, (short*) pOutput, frameCount) == 0) {
        EXIT_WITH_MESSAGE_ON_COND((ma_event_signal(&stop_audio_cond) != MA_SUCCESS), "Could not send signal to main thread!");
    }

    (void)pInput;
}

int main(int argc, char** argv) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    FAIL_WITH_MESSAGE_ON_COND(FAILED(CoInitialize(NULL)), "CoInitialize failed!");
#endif
    FAIL_WITH_MESSAGE_ON_COND(!oswrapper_audio_init(), "Could not initialise oswrapper_audio!");
    /* Allocate memory for an OSWrapper_audio_spec */
    OSWrapper_audio_spec* audio_spec = (OSWrapper_audio_spec*) calloc(1, sizeof(OSWrapper_audio_spec));
    FAIL_WITH_MESSAGE_ON_COND((audio_spec == NULL), "calloc failed!");
    /* Hint the desired output format */
    audio_spec->sample_rate = SAMPLE_RATE;
    audio_spec->channel_count = CHANNEL_COUNT;
    audio_spec->bits_per_channel = BITS_PER_CHANNEL;
    audio_spec->audio_type = AUDIO_FORMAT;
    /* Or set these values to zero to use the input format's values.
    The values in audio_spec will always be set to the output format's values
    after initialising an OSWrapper_audio_spec. */
    /* Load audio with desired format */
    int did_load_audio = oswrapper_audio_load_from_path(argc < 2 ? "noise.wav" : argv[argc - 1], audio_spec);
    FAIL_WITH_MESSAGE_ON_COND((did_load_audio == 0), "oswrapper_audio_load_from_path failed!");
    /* audio_spec now contains the output format values. */
    /* miniaudio: Find a suitable output format */
    ma_format format;

    if (audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
        FAIL_WITH_MESSAGE_ON_COND((audio_spec->bits_per_channel != 32), "Only 32 bit floating point PCM is supported with miniaudio!");
        format = ma_format_f32;
    } else {
        switch (audio_spec->bits_per_channel) {
        case 32:
            format = ma_format_s32;
            break;

        case 24:
            format = ma_format_s24;
            break;

        case 16:
            format = ma_format_s16;
            break;

        default:
            puts("No suitable output format for miniaudio!");
            return EXIT_FAILURE;
        }
    }

    /* miniaudio: Create a suitable device config */
    ma_device_config device_config;
    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format   = format;
    device_config.playback.channels = audio_spec->channel_count;
    device_config.sampleRate        = audio_spec->sample_rate;
    device_config.dataCallback      = data_callback;
    device_config.pUserData         = audio_spec;
    /* miniaudio: Initialise the device */
    ma_device device;
    FAIL_WITH_MESSAGE_ON_COND((ma_device_init(NULL, &device_config, &device) != MA_SUCCESS), "Failed to open miniaudio playback device!");
    /* miniaudio: Initialise the "stop playback" condition */
    FAIL_WITH_MESSAGE_ON_COND((ma_event_init(&stop_audio_cond) != MA_SUCCESS), "Failed to initialise miniaudio event!");
    /* miniaudio: Start the device */
    FAIL_WITH_MESSAGE_ON_COND((ma_device_start(&device) != MA_SUCCESS), "Failed to start miniaudio playback device!");
    /* Wait until audio playback is done */
    puts("Playing sound...");
    ma_event_wait(&stop_audio_cond);
#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
    /* Print current location for test purposes */
    OSWRAPPER_AUDIO_SEEK_TYPE pos = 0;
    int didGetPos = oswrapper_audio_get_pos(audio_spec, &pos);
    FAIL_WITH_MESSAGE_ON_COND((!didGetPos), "Failed to get current position!");
    printf("End of sound position was %lli\n", pos);
#endif
    /* Replay sound */
    ma_device_stop(&device);
#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
    /* For testing purposes. Equivalent to oswrapper_audio_rewind */
    oswrapper_audio_seek(audio_spec, 0);
#else
    oswrapper_audio_rewind(audio_spec);
#endif
    FAIL_WITH_MESSAGE_ON_COND((ma_event_init(&stop_audio_cond) != MA_SUCCESS), "Failed to reinitialise miniaudio event!");
    FAIL_WITH_MESSAGE_ON_COND((ma_device_start(&device) != MA_SUCCESS), "Failed to restart miniaudio playback device!");
    puts("Playing sound again...");
    ma_event_wait(&stop_audio_cond);
    puts("Finished playing sound!");
    /* Cleanup */
    ma_device_uninit(&device);
    /* Free audio file */
    int did_close = oswrapper_audio_free_context(audio_spec);
    FAIL_WITH_MESSAGE_ON_COND((!did_close), "Failed to free sound context!");
    FAIL_WITH_MESSAGE_ON_COND(!oswrapper_audio_uninit(), "Could not uninitialise oswrapper_audio!");
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    CoUninitialize();
#endif
    return EXIT_SUCCESS;
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
