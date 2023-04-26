/*
This program uses oswrapper_audio to decode an audio file,
and encodes the decoded PCM data to a variety of file types using macOS APIs.

Usage: test_oswrapper_audio_mac_encoder (audio_file.ext) (optional wanted file format)
If no input is provided, it will decode and encode the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_audio_mac_encoder.c
*/

#define OSWRAPPER_AUDIO_STATIC
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/AudioFormat.h>
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

#ifndef TEST_PROGRAM_BUFFER_SIZE
#define TEST_PROGRAM_BUFFER_SIZE 4096
#endif

static OSStatus test_encoder_create_from_path(const char* path, AudioStreamBasicDescription* output_format, ExtAudioFileRef* audio_file, AudioFileTypeID file_type) {
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
        error = ExtAudioFileCreateNew(&path_fsref, path_cfstr, file_type, output_format, NULL, audio_file);
    }

#else
    error = ExtAudioFileCreateWithURL(path_url, file_type, output_format, NULL, 0, audio_file);
#endif
    CFRelease(path_url);
    CFRelease(path_cfstr);
    return error;
}

static OSWRAPPER_AUDIO_RESULT_TYPE create_desc(AudioStreamBasicDescription* desc, AudioFormatID format_id, unsigned long sample_rate, unsigned int channel_count, unsigned int bits_per_channel, OSWrapper_audio_type audio_type, OSWrapper_audio_endianness_type endianness_type) {
    desc->mFormatID = format_id;
    /* This may be set to 0 when creating compressed formats */
    desc->mSampleRate = format_id == kAudioFormatMPEG4AAC ? 0 : sample_rate;
    desc->mChannelsPerFrame = channel_count;

    if (format_id == kAudioFormatLinearPCM) {
        if (audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
            desc->mFormatFlags = kLinearPCMFormatFlagIsFloat;
        } else if (audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER) {
            desc->mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
        } else {
            puts("Unsupported audio format, was not float or integer!");
            return OSWRAPPER_AUDIO_RESULT_FAILURE;
        }

        desc->mFormatFlags |= kLinearPCMFormatFlagIsPacked;

        if (endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG) {
            desc->mFormatFlags |= kAudioFormatFlagIsBigEndian;
        }

        desc->mBitsPerChannel = bits_per_channel;
        /* kAudioFormatLinearPCM doesn't use packets */
        desc->mFramesPerPacket = 1;
        /* Bytes per channel * channels per frame */
        desc->mBytesPerFrame = (desc->mBitsPerChannel / 8) * desc->mChannelsPerFrame;
        /* Bytes per frame * frames per packet */
        desc->mBytesPerPacket = desc->mBytesPerFrame * desc->mFramesPerPacket;
    } else {
        if (format_id == kAudioFormatAppleLossless || format_id == kAudioFormatFLAC) {
            if (audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
                puts("Unsupported input audio format, was not integer!");
                return OSWRAPPER_AUDIO_RESULT_FAILURE;
            } else {
                switch (bits_per_channel) {
                case 16:
                    desc->mFormatFlags = kAppleLosslessFormatFlag_16BitSourceData;
                    break;

                case 20:
                    desc->mFormatFlags = kAppleLosslessFormatFlag_20BitSourceData;
                    break;

                case 24:
                    desc->mFormatFlags = kAppleLosslessFormatFlag_24BitSourceData;
                    break;

                case 32:
                    desc->mFormatFlags = kAppleLosslessFormatFlag_32BitSourceData;
                    break;

                default:
                    puts("Unsupported input bit depth!");
                    return OSWRAPPER_AUDIO_RESULT_FAILURE;
                }
            }
        } else {
            desc->mFormatFlags = kAudioFormatFlagsAreAllClear;
        }
    }

    UInt32 property_size = sizeof(AudioStreamBasicDescription);

    if (AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &property_size, desc)) {
        puts("Could not create valid ASBD from input properties!");
        return OSWRAPPER_AUDIO_RESULT_FAILURE;
    }

    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

/* Decodes and re-encodes a given audio file to a variety of file types */
int main(int argc, char** argv) {
    int returnVal = EXIT_FAILURE;
    FILE* output_file = NULL;
    ExtAudioFileRef ext_output_file = NULL;
    AudioFileTypeID file_type;
    AudioFormatID file_format;
    OSWrapper_audio_endianness_type endianness_type;
    OSWrapper_audio_spec* audio_spec = NULL;
    char* output_path = NULL;
    short* buffer = NULL;
    const char* path;
    const char* ext;

    if (argc > 2) {
        path = argv[argc - 2];
        ext = argv[argc - 1];
    } else {
        path = argc < 2 ? "noise.wav" : argv[argc - 1];
        ext = "m4a";
    }

    if (strcmp(ext, "m4a") == 0) {
        puts("Converting to m4a (lossy aac)");
        endianness_type = OSWRAPPER_AUDIO_ENDIANNESS_USE_SYSTEM_DEFAULT;
        file_type = kAudioFileM4AType;
        file_format = kAudioFormatMPEG4AAC;
    } else if (strcmp(ext, "alac") == 0) {
        puts("Converting to m4a (lossless alac)");
        ext = "m4a";
        endianness_type = OSWRAPPER_AUDIO_ENDIANNESS_USE_SYSTEM_DEFAULT;
        file_type = kAudioFileM4AType;
        file_format = kAudioFormatAppleLossless;
    } else if (strcmp(ext, "flac") == 0) {
        puts("Converting to flac");
        endianness_type = OSWRAPPER_AUDIO_ENDIANNESS_USE_SYSTEM_DEFAULT;
        file_type = kAudioFileFLACType;
        file_format = kAudioFormatFLAC;
    } else if (strcmp(ext, "wav") == 0) {
        puts("Converting to wav");
        endianness_type = OSWRAPPER_AUDIO_ENDIANNESS_LITTLE;
        file_type = kAudioFileWAVEType;
        file_format = kAudioFormatLinearPCM;
    } else if (strcmp(ext, "snd") == 0) {
        puts("Converting to snd");
        endianness_type = OSWRAPPER_AUDIO_ENDIANNESS_BIG;
        file_type = kAudioFileNextType;
        file_format = kAudioFormatLinearPCM;
    } else {
        /* Default to m4a */
        puts("Converting to m4a (lossy aac)");
        ext = "m4a";
        endianness_type = OSWRAPPER_AUDIO_ENDIANNESS_USE_SYSTEM_DEFAULT;
        file_type = kAudioFileM4AType;
        file_format = kAudioFormatMPEG4AAC;
    }

    size_t input_string_length = strlen(path);
    /* "." + extention */
    size_t output_ext_length = 1 + strlen(ext);

    if (!oswrapper_audio_init()) {
        puts("Could not initialise oswrapper_audio!");
        goto exit;
    }

    audio_spec = (OSWrapper_audio_spec*) calloc(1, sizeof(OSWrapper_audio_spec));

    if (audio_spec == NULL) {
        puts("calloc failed for OSWrapper_audio_spec!");
        goto exit;
    }

    output_path = (char*) malloc(input_string_length + output_ext_length);

    if (output_path == NULL) {
        puts("malloc failed for output path!");
        goto exit;
    }

    memcpy(output_path, path, input_string_length);
    output_path[input_string_length] = '.';
    memcpy(output_path + input_string_length + 1, ext, output_ext_length);
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
    audio_spec->endianness_type = endianness_type;

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
#ifdef __cplusplus
        AudioStreamBasicDescription input_format = { };
        AudioStreamBasicDescription output_format = { };
#else
        AudioStreamBasicDescription input_format = { 0 };
        AudioStreamBasicDescription output_format = { 0 };
#endif
        AudioConverterRef converter = NULL;
        UInt32 property_size = sizeof(AudioConverterRef);
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
        if (!create_desc(&input_format, kAudioFormatLinearPCM, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel, audio_spec->audio_type, audio_spec->endianness_type)) {
            goto audio_cleanup;
        }

        /* Output format */
        if (!create_desc(&output_format, file_format, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel, audio_spec->audio_type, audio_spec->endianness_type)) {
            goto audio_cleanup;
        }

        if (test_encoder_create_from_path(output_path, &output_format, &ext_output_file, file_type)) {
            puts("Couldn't open output file for encoding!");
            goto audio_cleanup;
        }

        if (ExtAudioFileSetProperty(ext_output_file, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription), &input_format)) {
            puts("Couldn't set input format for encoding!");
            goto audio_cleanup;
        }

        if (file_format == kAudioFormatMPEG4AAC && !ExtAudioFileGetProperty(ext_output_file, kExtAudioFileProperty_AudioConverter, &property_size, &converter) && converter != NULL) {
            /* Set encoding bitrate to 256kpbs */
            UInt32 bitrate = 256000;
            /* Failure is mostly harmless */
            AudioConverterSetProperty(converter, kAudioConverterEncodeBitRate, sizeof(bitrate), &bitrate);
            CFArrayRef converter_config = NULL;
            ExtAudioFileSetProperty(ext_output_file, kExtAudioFileProperty_ConverterConfig, sizeof(CFArrayRef), &converter_config);
        }

        while (1) {
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);

            if (this_iter == 0) {
                break;
            }

            output_buffer_list.mBuffers[0].mDataByteSize = this_iter * frame_size;

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
