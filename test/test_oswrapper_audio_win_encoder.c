/*
This program uses oswrapper_audio to decode an audio file,
and encodes the decoded PCM data to WAV using Windows APIs.

Usage: test_oswrapper_audio_win_encoder (audio_file.ext)
If no input is provided, it will decode and encode the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_audio_win_encoder.c
*/

#define OSWRAPPER_AUDIO_STATIC
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#include <objbase.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

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

#ifndef TEST_PROGRAM_BUFFER_SIZE
#define TEST_PROGRAM_BUFFER_SIZE (4096 * 16)
#endif

#ifdef OSWRAPPER_AUDIO_PATH_MAX
#define DEMO_WIN_MAX_PATH OSWRAPPER_AUDIO_PATH_MAX
#else
#define DEMO_WIN_MAX_PATH MAX_PATH
#endif

#if !defined(DEMO_WIN_CONVERT_TO_M4A) && !defined(DEMO_WIN_CONVERT_TO_WAV) && !defined(DEMO_WIN_CONVERT_TO_MP3)
#define DEMO_WIN_CONVERT_TO_WAV
#endif

#ifdef DEMO_WIN_CONVERT_TO_M4A
#define DEMO_WIN_EXT ".m4a"
#define DEMO_WIN_CONV_FORMAT &MFAudioFormat_AAC
#elif defined(DEMO_WIN_CONVERT_TO_WAV)
#define DEMO_WIN_EXT ".wav"
#define DEMO_WIN_CONV_FORMAT &MFAudioFormat_PCM
#elif defined(DEMO_WIN_CONVERT_TO_MP3)
#define DEMO_WIN_EXT ".mp3"
#define DEMO_WIN_CONV_FORMAT &MFAudioFormat_MP3
#else
#error No format defined
#endif

static OSWRAPPER_AUDIO_RESULT_TYPE make_sink_writer_from_path(const char* path, IMFSinkWriter** writer) {
    HRESULT result;
    /* TODO Ugly hack */
    wchar_t path_buffer[OSWRAPPER_AUDIO_PATH_MAX];
    result = MultiByteToWideChar(CP_UTF8, 0, path, -1, path_buffer, DEMO_WIN_MAX_PATH);

    if (SUCCEEDED(result)) {
        result = MFCreateSinkWriterFromURL(path_buffer, NULL, NULL, writer);

        if (SUCCEEDED(result)) {
            return OSWRAPPER_AUDIO_RESULT_SUCCESS;
        }
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

#define DEMO_ENC_XSTR(X) DEMO_ENC_STR(X)
#define DEMO_ENC_STR(X) #X

#define DEMO_WIN_ENC__END_FAIL_FALSE(cond) if(!cond) { puts(DEMO_ENC_XSTR(cond) " failed!"); goto exit; }
#define DEMO_WIN_ENC__END_FAIL(hres) DEMO_WIN_ENC__END_FAIL_FALSE(SUCCEEDED(hres))

/* Decodes and re-encodes a given audio file to WAV */
int main(int argc, char** argv) {
    HRESULT result = CoInitialize(NULL);

    if (FAILED(result)) {
        puts("CoInitialize failed!");
        return EXIT_FAILURE;
    }

    if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE))) {
        CoUninitialize();
        puts("MFStartup failed!");
        return EXIT_FAILURE;
    }

    int returnVal = EXIT_FAILURE;
    FILE* output_file = NULL;
    IMFSinkWriter* writer;
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

    output_path = (char*) malloc(input_string_length + sizeof(DEMO_WIN_EXT));

    if (output_path == NULL) {
        puts("malloc failed for output path!");
        goto exit;
    }

    memcpy(output_path, path, input_string_length);
    memcpy(output_path + input_string_length, DEMO_WIN_EXT, sizeof(DEMO_WIN_EXT));
    output_file = fopen(output_path, "rb");

    if (output_file != NULL) {
        fclose(output_file);
        output_file = NULL;
        printf("Output file %s already exists!\n", output_path);
        goto exit;
    }

    if (make_sink_writer_from_path(output_path, &writer) != OSWRAPPER_AUDIO_RESULT_SUCCESS) {
        printf("Could not create sink writer for output file %s!\n", output_path);
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
            puts("Input format: floating point PCM\n");
        } else {
            puts("Input format: integer PCM\n");
        }

        if (audio_spec->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG) {
            puts("Big-endian\n");
        } else {
            puts("Little-endian\n");
        }

        size_t frame_size = (audio_spec->bits_per_channel / 8) * (audio_spec->channel_count);
        buffer = (short*) calloc(TEST_PROGRAM_BUFFER_SIZE, frame_size);
        size_t frames_done = 0;
        LONGLONG output_pos = 0;
        DWORD audio_stream_index = 0;
        /* Output stream format */
        IMFMediaType* output_media_type;
        DEMO_WIN_ENC__END_FAIL(MFCreateMediaType(&output_media_type));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetGUID(output_media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetGUID(output_media_type, &MF_MT_SUBTYPE, DEMO_WIN_CONV_FORMAT));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(output_media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(output_media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(output_media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
#ifdef DEMO_WIN_CONVERT_TO_M4A
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(output_media_type, &MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 128 * 1000 / 8));
#elif defined(DEMO_WIN_CONVERT_TO_MP3)
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(output_media_type, &MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 320 * 1000 / 8));
#endif
        DEMO_WIN_ENC__END_FAIL(IMFSinkWriter_AddStream(writer, output_media_type, &audio_stream_index));
        IMFMediaType_Release(output_media_type);
        /* Input stream format */
        IMFMediaType* input_media_type;
        DEMO_WIN_ENC__END_FAIL(MFCreateMediaType(&input_media_type));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetGUID(input_media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetGUID(input_media_type, &MF_MT_SUBTYPE, audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT ? &MFAudioFormat_Float : &MFAudioFormat_PCM));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(input_media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(input_media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        DEMO_WIN_ENC__END_FAIL(IMFMediaType_SetUINT32(input_media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
        DEMO_WIN_ENC__END_FAIL(IMFSinkWriter_SetInputMediaType(writer, audio_stream_index, input_media_type, NULL));
        IMFMediaType_Release(input_media_type);
        /* Initialise sink writer */
        DEMO_WIN_ENC__END_FAIL(IMFSinkWriter_BeginWriting(writer));
        /* Create IMFSample to use when encoding */
        IMFSample* sample = NULL;
        IMFMediaBuffer* media_buffer = NULL;
        DEMO_WIN_ENC__END_FAIL(MFCreateSample(&sample));
        DEMO_WIN_ENC__END_FAIL(MFCreateMemoryBuffer(TEST_PROGRAM_BUFFER_SIZE * frame_size, &media_buffer));
        DEMO_WIN_ENC__END_FAIL(IMFSample_AddBuffer(sample, media_buffer));

        if (buffer == NULL) {
            puts("calloc failed for audio decoding buffer!");
            goto audio_cleanup;
        }

        while (1) {
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);
            size_t this_multi = this_iter * frame_size;

            if (this_iter == 0) {
                break;
            }

            DWORD current_length;
            BYTE* sample_audio_data = NULL;
            result = IMFMediaBuffer_Lock(media_buffer, &sample_audio_data, &current_length, NULL);

            if (SUCCEEDED(result)) {
                memcpy(sample_audio_data, buffer, this_multi);
                IMFMediaBuffer_Unlock(media_buffer);
                result = IMFMediaBuffer_SetCurrentLength(media_buffer, this_multi);

                if (SUCCEEDED(result)) {
                    /* TODO This is probably wrong */
                    LONGLONG this_dur = MFllMulDiv(this_iter, 10000000ULL, audio_spec->sample_rate, 0);
                    result = IMFSample_SetSampleDuration(sample, this_dur);

                    if (FAILED(result)) {
                        puts("IMFSample_SetSampleDuration failed!");
                    }

                    result = IMFSample_SetSampleTime(sample, output_pos);
                    output_pos += this_dur;

                    if (FAILED(result)) {
                        puts("IMFSample_SetSampleTime failed!");
                    }

                    result = IMFSinkWriter_WriteSample(writer, audio_stream_index, sample);

                    if (FAILED(result)) {
                        puts("IMFSinkWriter_WriteSample failed!");
                    }
                } else {
                    puts("IMFMediaBuffer_SetCurrentLength failed!");
                }
            } else {
                puts("IMFMediaBuffer_Lock failed!");
            }

            frames_done += this_iter;
        }

        printf("Encoded %zu frames of audio, with frame size %zu\n", frames_done, frame_size);
        returnVal = EXIT_SUCCESS;
audio_cleanup:
        IMFSinkWriter_Finalize(writer);
        IMFSinkWriter_Release(writer);

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

    if (FAILED(MFShutdown())) {
        puts("Could not uninitialise MF!");
        returnVal = EXIT_FAILURE;
    }

exit:
    CoUninitialize();

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
