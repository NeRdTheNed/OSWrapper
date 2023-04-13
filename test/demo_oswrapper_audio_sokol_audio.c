/*
This program uses oswrapper_audio to decode and play an audio file twice,
using sokol_audio for sound output.

Usage: demo_oswrapper_audio_sokol_audio (audio_file.ext)
If no input is provided, it will play the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/demo_oswrapper_audio_sokol_audio.c
*/

#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#ifdef __APPLE__
/* Sleep function */
#include <unistd.h>

#define SAUDIO_OSX_USE_SYSTEM_HEADERS
#endif
#define SOKOL_AUDIO_IMPL
#include "sokol_audio.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* Sleep function */
#include <windows.h>
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
#define BITS_PER_CHANNEL 32
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT
#else
#define SAMPLE_RATE 0
#define CHANNEL_COUNT 0
#define BITS_PER_CHANNEL 32
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT
#endif

#define FLOAT_BUFFER_SIZE 0x4000
#define SLEEP_TIME 100

#define FAIL_WITH_MESSAGE_ON_COND(cond, message) if ((cond)) { puts(message); return EXIT_FAILURE; }

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
    /* sokol_audio: Assert that we're decoding to 32 bit float */
    FAIL_WITH_MESSAGE_ON_COND((audio_spec->audio_type != OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT || audio_spec->bits_per_channel != 32), "Only 32 bit floating point PCM is supported with sokol_audio!");
    /* sokol_audio: Create a suitable config */
#ifdef __cplusplus
    saudio_desc format = { };
#else
    saudio_desc format = { 0 };
#endif
    format.num_channels = audio_spec->channel_count;
    format.sample_rate = audio_spec->sample_rate;
    /* sokol_audio: Initialise sokol_audio */
    saudio_setup(&format);
    FAIL_WITH_MESSAGE_ON_COND((saudio_isvalid() == false), "Failed to initialise sokol_audio!");
    FAIL_WITH_MESSAGE_ON_COND((saudio_sample_rate() != (int) audio_spec->sample_rate), "Output sample rate was not the same as requested sample rate!");
    FAIL_WITH_MESSAGE_ON_COND((saudio_channels() != (int) audio_spec->channel_count), "Output sample rate was not the same as requested sample rate!");
    float float_buffer[FLOAT_BUFFER_SIZE];
    bool first_time = true;
    puts("Playing sound...");

    /* sokol_audio: Push samples until the file has been decoded twice */
    while (true) {
        const int num_frames = saudio_expect();

        if (num_frames > 0) {
            FAIL_WITH_MESSAGE_ON_COND((saudio_sample_rate() != (int) audio_spec->sample_rate), "Output sample rate was not the same as requested sample rate!");
            FAIL_WITH_MESSAGE_ON_COND((saudio_channels() != (int) audio_spec->channel_count), "Output sample rate was not the same as requested sample rate!");
            const int num_samples = num_frames > (FLOAT_BUFFER_SIZE / saudio_channels()) ? FLOAT_BUFFER_SIZE / saudio_channels() : num_frames;
            size_t decoded_samples = oswrapper_audio_get_samples(audio_spec, (short*) &float_buffer, num_samples);

            if (decoded_samples == 0) {
                if (first_time) {
                    puts("Playing sound again...");
                    oswrapper_audio_rewind(audio_spec);
                    first_time = false;
                } else {
                    puts("Finished playing sound!");
                    break;
                }
            } else {
                saudio_push(float_buffer, decoded_samples);
            }
        }

#ifdef _WIN32
        Sleep(SLEEP_TIME);
#elif defined(__APPLE__)
        usleep(SLEEP_TIME * 1000);
#endif
    }

    /* Cleanup */
    saudio_shutdown();
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
