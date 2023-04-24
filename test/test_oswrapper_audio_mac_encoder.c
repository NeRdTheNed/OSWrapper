/*
This program uses oswrapper_audio to decode an audio file,
and encode the decoded PCM data to M4A (or WAV if you define DEMO_CONVERT_TO_WAV).

Usage: test_oswrapper_audio_mac_encoder (audio_file.ext)
If no input is provided, it will decode and encode the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_audio_mac_encoder.c
*/

#define OSWRAPPER_AUDIO_STATIC
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/ExtendedAudioFile.h>

#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
#include <CoreServices/CoreServices.h>
#endif

#include <stdio.h>
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

#if !defined(DEMO_CONVERT_TO_WAV) && !defined(DEMO_CONVERT_TO_M4A)
#define DEMO_CONVERT_TO_M4A
#endif

#ifdef DEMO_CONVERT_TO_WAV
#define DEMO_AUDIO_FILE_TYPE kAudioFileWAVEType
#define DEMO_AUDIO_FILL_OUTPUT_METHOD(desc, sample_rate, channel_count, bits_per_channel, audio_type) create_pcm_desc(desc, sample_rate, channel_count, bits_per_channel, audio_type)
#define DEMO_AUDIO_FILE_EXT ".wav"
#elif defined(DEMO_CONVERT_TO_M4A)
#define DEMO_AUDIO_FILE_TYPE kAudioFileM4AType
#define DEMO_AUDIO_FILL_OUTPUT_METHOD(desc, sample_rate, channel_count, bits_per_channel, audio_type) create_m4a_desc(desc, sample_rate, channel_count)
#define DEMO_AUDIO_FILE_EXT ".m4a"
#else
#error No format defined
#endif

#define TEST_PROGRAM_BUFFER_SIZE 1024

static OSStatus test_encoder_create_from_path(const char* path, AudioStreamBasicDescription* output_format, ExtAudioFileRef* audio_file) {
    OSStatus error;
    CFStringRef path_cfstr;
    CFURLRef path_url;
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    FSRef path_fsref;
    const UInt8* base_dir;
#endif
    path_cfstr = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
    path_url = CFURLCreateWithFileSystemPath(NULL, path_cfstr, kCFURLPOSIXPathStyle, false);
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    /* TODO Assumes a relative path */
    base_dir = (const UInt8*) "./";
    Boolean is_dir;
    error = FSPathMakeRef(base_dir, &path_fsref, &is_dir);

    if (!error && is_dir) {
        error = ExtAudioFileCreateNew(&path_fsref, path_cfstr, DEMO_AUDIO_FILE_TYPE, output_format, NULL, audio_file);
    }

#else
    error = ExtAudioFileCreateWithURL(path_url, DEMO_AUDIO_FILE_TYPE, output_format, NULL, 0, audio_file);
#endif
    CFRelease(path_url);
    CFRelease(path_cfstr);
    return error;
}

static OSWRAPPER_AUDIO_RESULT_TYPE create_pcm_desc(AudioStreamBasicDescription* desc, unsigned long sample_rate, unsigned int channel_count, unsigned int bits_per_channel, OSWrapper_audio_type audio_type) {
    desc->mFormatID = kAudioFormatLinearPCM;

    if (audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
        desc->mFormatFlags = kLinearPCMFormatFlagIsFloat;
    } else if (audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER) {
        desc->mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
    } else {
        puts("Unsupported audio format, was not float or integer!");
        return OSWRAPPER_AUDIO_RESULT_FAILURE;
    }

    desc->mFormatFlags |= kLinearPCMFormatFlagIsPacked
#if defined(__ppc64__) || defined(__ppc__)
                          | kAudioFormatFlagIsBigEndian
#endif
                          ;
    desc->mSampleRate = sample_rate;
    desc->mBitsPerChannel = bits_per_channel;
    desc->mChannelsPerFrame = channel_count;
    /* kAudioFormatLinearPCM doesn't use packets */
    desc->mFramesPerPacket = 1;
    /* Bytes per channel * channels per frame */
    desc->mBytesPerFrame = (desc->mBitsPerChannel / 8) * desc->mChannelsPerFrame;
    /* Bytes per frame * frames per packet */
    desc->mBytesPerPacket = desc->mBytesPerFrame * desc->mFramesPerPacket;
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

#ifdef DEMO_CONVERT_TO_M4A
static OSWRAPPER_AUDIO_RESULT_TYPE create_m4a_desc(AudioStreamBasicDescription* desc, unsigned long sample_rate, unsigned int channel_count) {
    desc->mFormatID = kAudioFormatMPEG4AAC;
    desc->mFormatFlags = kAudioFormatFlagsAreAllClear;
    desc->mSampleRate = sample_rate;
    desc->mChannelsPerFrame = channel_count;
    /* Must be 1024 */
    desc->mFramesPerPacket = TEST_PROGRAM_BUFFER_SIZE;
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}
#endif

/* Decodes and re-encodes a given audio file to M4A (or WAV if you define DEMO_CONVERT_TO_WAV) */
int main(int argc, char** argv) {
    int returnVal = EXIT_FAILURE;
    FILE* output_file = NULL;
    ExtAudioFileRef ext_output_file = NULL;
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

    output_path = (char*) malloc(input_string_length + sizeof(DEMO_AUDIO_FILE_EXT));

    if (output_path == NULL) {
        puts("malloc failed for output path!");
        goto exit;
    }

    memcpy(output_path, path, input_string_length);
    memcpy(output_path + input_string_length, DEMO_AUDIO_FILE_EXT, sizeof(DEMO_AUDIO_FILE_EXT));
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

    if (oswrapper_audio_load_from_path(path, audio_spec)) {
        printf("Path: %s\nOutput path: %s\nSample rate: %lu\nChannels: %d\nBit depth: %d\n", path, output_path, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel);

        if (audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
            puts("Input format: floating point PCM\n");
        } else {
            puts("Input format: integer PCM\n");
        }

        size_t frame_size = (audio_spec->bits_per_channel / 8) * (audio_spec->channel_count);
        buffer = (short*) calloc(TEST_PROGRAM_BUFFER_SIZE, frame_size);
        size_t frames_done = 0;
#ifdef __cplusplus
        AudioStreamBasicDescription input_format = { };
        AudioStreamBasicDescription output_format = { };
#else
        AudioStreamBasicDescription input_format = { 0 };
        AudioStreamBasicDescription output_format = { 0 };
#endif
        AudioBufferList output_buffer_list;

        if (buffer == NULL) {
            puts("calloc failed for audio decoding buffer!");
            goto audio_cleanup;
        }

        /* Output buffer list */
        output_buffer_list.mNumberBuffers = 1;
        output_buffer_list.mBuffers[0].mNumberChannels = audio_spec->channel_count;
        output_buffer_list.mBuffers[0].mData = buffer;

        /* Input PCM format */
        if (!create_pcm_desc(&input_format, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel, audio_spec->audio_type)) {
            goto audio_cleanup;
        }

        /* Output format */
        if (!DEMO_AUDIO_FILL_OUTPUT_METHOD(&output_format, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel, audio_spec->audio_type)) {
            goto audio_cleanup;
        }

        if (test_encoder_create_from_path(output_path, &output_format, &ext_output_file)) {
            puts("Couldn't open output file for encoding!");
            goto audio_cleanup;
        }

        if (ExtAudioFileSetProperty(ext_output_file, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription), &input_format)) {
            puts("Couldn't set input format for encoding!");
            goto audio_cleanup;
        }

        while (1) {
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);

            if (this_iter == 0) {
                break;
            }

            output_buffer_list.mBuffers[0].mDataByteSize = this_iter * audio_spec->channel_count * frame_size;

            if (ExtAudioFileWrite(ext_output_file, this_iter, &output_buffer_list)) {
                printf("Error writing frame at position %zu!\n", frames_done);
            }

            frames_done += this_iter;
        }

        ExtAudioFileDispose(ext_output_file);
        ext_output_file = NULL;
        printf("Encoded %zu frames of audio, with frame size %zu\n", frames_done, frame_size);
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

    if (ext_output_file != NULL) {
        ExtAudioFileDispose(ext_output_file);
        ext_output_file = NULL;
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
