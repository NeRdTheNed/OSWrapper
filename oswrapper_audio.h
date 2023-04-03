/*
OSWrapper audio: Load audio files with the built in OS audio decoders.

Usage:
TODO, see comments on API function declarations, and demo_oswrapper_audio_mac.c in test folder.

Make sure to call oswrapper_audio_init() before using the library.
Call oswrapper_audio_uninit() after you no longer need to use oswrapper_audio.
*/

#ifndef OSWRAPPER_INCLUDE_OSWRAPPER_AUDIO_H
#define OSWRAPPER_INCLUDE_OSWRAPPER_AUDIO_H
#include <stddef.h>

#ifndef OSWRAPPER_AUDIO_DEF
#ifdef OSWRAPPER_AUDIO_STATIC
#define OSWRAPPER_AUDIO_DEF static
#else
#define OSWRAPPER_AUDIO_DEF extern
#endif
#endif /* OSWRAPPER_AUDIO_DEF */

/* You can make these functions return actual booleans if you want */
#ifndef OSWRAPPER_AUDIO_RESULT_TYPE
#define OSWRAPPER_AUDIO_RESULT_TYPE int
#endif

#ifndef OSWRAPPER_AUDIO_RESULT_SUCCESS
#define OSWRAPPER_AUDIO_RESULT_SUCCESS 1
#endif
#ifndef OSWRAPPER_AUDIO_RESULT_FAILURE
#define OSWRAPPER_AUDIO_RESULT_FAILURE 0
#endif

#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
/* Unstable-ish API */
#ifndef OSWRAPPER_AUDIO_SEEK_TYPE
#define OSWRAPPER_AUDIO_SEEK_TYPE long long
#endif
#endif /* OSWRAPPER_AUDIO_EXPERIMENTAL */

/* Stable-ish API */

/* The created audio context.
The values can be set before creating an audio context
with the oswrapper_audio_load_from_ functions,
which will be treated as hints for choosing the output format for decoding.
The values should be treated as read-only once created, otherwise things will break.
Don't use the internal_data member,
it's for storing the platform-specific decoding context. */
typedef struct OSWrapper_audio_spec {
    void* internal_data;
    unsigned long sample_rate;
    unsigned int channel_count;
    unsigned int bits_per_channel;
} OSWrapper_audio_spec;

/* Call oswrapper_audio_init() before using the library,
and call oswrapper_audio_uninit() after you're done using the library. */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_init(void);
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_uninit(void);

/* Free resources associated with the given OSWrapper_audio_spec.
Returns 1 on success, or 0 on failure. */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_free_context(OSWrapper_audio_spec* audio);

/* Load a sound file from memory.
The passed memory may not be copied, and is not freed (you retain full control over it).
Therefore, the memory must stay valid until after you call oswrapper_audio_free_context.
You can set the values on the passed OSWrapper_audio_spec,
which will be treated as hints for choosing the output format for decoding.
The initialised audio context is returned in the passed OSWrapper_audio_spec,
which will also contain information about the output format (channels, sample rate etc.)
Returns 1 on success, or 0 on failure. */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_memory(const unsigned char* data, size_t data_size, OSWrapper_audio_spec* audio);
#ifndef OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH
/* Load a sound file from the given path.
You can set the values on the passed OSWrapper_audio_spec,
which will be treated as hints for choosing the output format for decoding.
The initialised audio context is returned in the passed OSWrapper_audio_spec,
which will also contain information about the output format (channels, sample rate etc.)
Returns 1 on success, or 0 on failure. */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_path(const char* path, OSWrapper_audio_spec* audio);
#endif /* OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
/* Unstable-ish API */
/* Sets the value pointed to by pos to the current position.
Returns 1 on success, or 0 on failure. */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_get_pos(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE* pos);
/* Seek to the given position. */
OSWRAPPER_AUDIO_DEF void oswrapper_audio_seek(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE pos);
#endif /* OSWRAPPER_AUDIO_EXPERIMENTAL */
/* Seek to the start of the audio context. */
OSWRAPPER_AUDIO_DEF void oswrapper_audio_rewind(OSWrapper_audio_spec* audio);

/* Write decoded audio samples to the given buffer. The return value is the amount of samples written. */
OSWRAPPER_AUDIO_DEF size_t oswrapper_audio_get_samples(OSWrapper_audio_spec* audio, short* buffer, size_t frames_to_do);

#ifdef OSWRAPPER_AUDIO_IMPLEMENTATION
#ifndef OSWRAPPER_AUDIO_NO_INCLUDE_STDLIB
#include <stdlib.h>
#endif
#ifndef OSWRAPPER_AUDIO_NO_INCLUDE_STRING
#include <string.h>
#endif

#ifndef OSWRAPPER_AUDIO_MALLOC
#define OSWRAPPER_AUDIO_MALLOC(x) malloc(x)
#endif /* OSWRAPPER_AUDIO_MALLOC */
#ifndef OSWRAPPER_AUDIO_FREE
#define OSWRAPPER_AUDIO_FREE(x) free(x)
#endif /* OSWRAPPER_AUDIO_FREE */
#ifndef OSWRAPPER_AUDIO_MEMCPY
#define OSWRAPPER_AUDIO_MEMCPY(x, y, amount) memcpy(x, y, amount)
#endif /* OSWRAPPER_AUDIO_MEMCPY */

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
#if !defined(OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL) && !defined(OSWRAPPER_AUDIO_NO_USE_AUDIOTOOLBOX_IMPL)
#define OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL
#endif /* !defined(OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL) && !defined(OSWRAPPER_AUDIO_NO_USE_AUDIOTOOLBOX_IMPL) */
#endif /* defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4 */
#endif /* __APPLE__ */

#ifdef OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL
/* Start macOS AudioToolbox implementation */
#include <AudioToolbox/AudioToolbox.h>

#include <AvailabilityMacros.h>

#if !defined(MAC_OS_X_VERSION_10_6) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
#define kAudioFileReadPermission 0x01
#endif

typedef struct oswrapper_audio__callback_data_mac {
    size_t data_size;
    const unsigned char* data;
} oswrapper_audio__callback_data_mac;

typedef struct oswrapper_audio__internal_data_mac {
    AudioFileID audio_file;
    ExtAudioFileRef audio_file_ext;
    oswrapper_audio__callback_data_mac* callback_data;
} oswrapper_audio__internal_data_mac;

static OSStatus oswrapper_audio__audio_file_read_callback(void* inClientData, SInt64 inPosition, UInt32 requestCount, void* buffer, UInt32* actualCount) {
    size_t bytes_read;
    oswrapper_audio__callback_data_mac* callback_data = (oswrapper_audio__callback_data_mac*) inClientData;

    if (inPosition < (SInt64) callback_data->data_size) {
        size_t bytes_available = callback_data->data_size - inPosition;
        bytes_read = requestCount <= bytes_available ? requestCount : bytes_available;
        OSWRAPPER_AUDIO_MEMCPY((buffer), (callback_data->data + inPosition), (bytes_read));
    } else {
        bytes_read = 0;
    }

    if (actualCount != NULL) {
        *actualCount = bytes_read;
    }

    return 0;
}

static SInt64 oswrapper_audio__audio_file_get_size_callback(void* inClientData) {
    oswrapper_audio__callback_data_mac* callback_data = (oswrapper_audio__callback_data_mac*) inClientData;
    return callback_data->data_size;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_init(void) {
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_uninit(void) {
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_free_context(OSWrapper_audio_spec* audio) {
    OSStatus error;
    oswrapper_audio__internal_data_mac* internal_data = (oswrapper_audio__internal_data_mac*) audio->internal_data;
    error = ExtAudioFileDispose(internal_data->audio_file_ext);

    if (!error) {
        error = AudioFileClose(internal_data->audio_file);

        if (!error) {
            /* callback_data is only used for in-memory decoding, so this is expected to be NULL when decoding files. */
#ifndef OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH
            if (internal_data->callback_data != NULL)
#endif
            {
                OSWRAPPER_AUDIO_FREE(internal_data->callback_data);
            }

            OSWRAPPER_AUDIO_FREE(audio->internal_data);
            return OSWRAPPER_AUDIO_RESULT_SUCCESS;
        }
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

static OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio__load_from_open(AudioFileID audio_file, oswrapper_audio__callback_data_mac* callback_data, OSWrapper_audio_spec* audio) {
    OSStatus error;
    ExtAudioFileRef audio_file_ext;
    error = ExtAudioFileWrapAudioFileID(audio_file, false, &audio_file_ext);

    if (!error) {
        AudioStreamBasicDescription input_file_format;
        UInt32 property_size = sizeof(AudioStreamBasicDescription);
        error = ExtAudioFileGetProperty(audio_file_ext, kExtAudioFileProperty_FileDataFormat, &property_size, &input_file_format);

        if (!error) {
            AudioStreamBasicDescription output_format;
            output_format.mFormatID = kAudioFormatLinearPCM;
            output_format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked
#if defined(__ppc64__) || defined(__ppc__)
                                         | kAudioFormatFlagIsBigEndian
#endif
                                         ;
            /* Use hinted sample rate */
            output_format.mSampleRate = audio->sample_rate == 0 ? input_file_format.mSampleRate : audio->sample_rate;

            /* Sanity check */
            if (output_format.mSampleRate == 0) {
                output_format.mSampleRate = 44100;
            }

            /* Use hinted bits per channel */
            output_format.mBitsPerChannel = audio->bits_per_channel == 0 ? input_file_format.mBitsPerChannel : audio->bits_per_channel;

            /* Sanity check */
            if (output_format.mBitsPerChannel == 0) {
                output_format.mBitsPerChannel = 16;
            }

            /* Use hinted channels */
            output_format.mChannelsPerFrame = audio->channel_count == 0 ? input_file_format.mChannelsPerFrame : audio->channel_count;

            /* Sanity check */
            if (output_format.mChannelsPerFrame == 0) {
                output_format.mChannelsPerFrame = 2;
            }

            /* kAudioFormatLinearPCM doesn't use packets */
            output_format.mFramesPerPacket = 1;
            /* Bytes per channel * channels per frame */
            output_format.mBytesPerFrame = (output_format.mBitsPerChannel / 8) * output_format.mChannelsPerFrame;
            /* Bytes per frame * frames per packet */
            output_format.mBytesPerPacket = output_format.mBytesPerFrame * output_format.mFramesPerPacket;
            error = ExtAudioFileSetProperty(audio_file_ext, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription), &output_format);

            if (!error) {
                oswrapper_audio__internal_data_mac* internal_data = (oswrapper_audio__internal_data_mac*) OSWRAPPER_AUDIO_MALLOC(sizeof(oswrapper_audio__internal_data_mac));

                if (internal_data != NULL) {
                    audio->sample_rate = output_format.mSampleRate;
                    audio->bits_per_channel = output_format.mBitsPerChannel;
                    audio->channel_count = output_format.mChannelsPerFrame;
                    audio->internal_data = (void*) internal_data;
                    internal_data->audio_file = audio_file;
                    internal_data->audio_file_ext = audio_file_ext;
                    internal_data->callback_data = callback_data;
                    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
                }
            }
        }

        ExtAudioFileDispose(audio_file_ext);
    }

    AudioFileClose(audio_file);
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_memory(const unsigned char* data, size_t data_size, OSWrapper_audio_spec* audio) {
    oswrapper_audio__callback_data_mac* callback_data = (oswrapper_audio__callback_data_mac*) OSWRAPPER_AUDIO_MALLOC(sizeof(oswrapper_audio__callback_data_mac));

    if (callback_data != NULL) {
        AudioFileID audio_file;
        OSStatus error;
        callback_data->data = data;
        callback_data->data_size = data_size;
        error = AudioFileOpenWithCallbacks((void*) callback_data, oswrapper_audio__audio_file_read_callback, NULL, oswrapper_audio__audio_file_get_size_callback, NULL, 0, &audio_file);

        if (!error && oswrapper_audio__load_from_open(audio_file, callback_data, audio)) {
            return OSWRAPPER_AUDIO_RESULT_SUCCESS;
        }

        OSWRAPPER_AUDIO_FREE(callback_data);
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

#ifndef OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_path(const char* path, OSWrapper_audio_spec* audio) {
    AudioFileID audio_file;
    OSStatus error;
    CFStringRef path_cfstr;
    CFURLRef path_url;
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    FSRef path_fsref;
#endif
    path_cfstr = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
    path_url = CFURLCreateWithFileSystemPath(NULL, path_cfstr, kCFURLPOSIXPathStyle, false);
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    CFURLGetFSRef(path_url, &path_fsref);
    error = AudioFileOpen(&path_fsref, kAudioFileReadPermission, 0, &audio_file);
#else
    error = AudioFileOpenURL(path_url, kAudioFileReadPermission, 0, &audio_file);
#endif
    CFRelease(path_url);
    CFRelease(path_cfstr);

    if (!error && oswrapper_audio__load_from_open(audio_file, NULL, audio)) {
        return OSWRAPPER_AUDIO_RESULT_SUCCESS;
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
/* Unstable-ish API */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_get_pos(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE* pos) {
    OSStatus error;
    oswrapper_audio__internal_data_mac* internal_data = (oswrapper_audio__internal_data_mac*) audio->internal_data;
    error = ExtAudioFileTell(internal_data->audio_file_ext, pos);
    return !error ? OSWRAPPER_AUDIO_RESULT_SUCCESS : OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF void oswrapper_audio_seek(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE pos) {
    oswrapper_audio__internal_data_mac* internal_data = (oswrapper_audio__internal_data_mac*) audio->internal_data;
    ExtAudioFileSeek(internal_data->audio_file_ext, pos);
}
#endif /* OSWRAPPER_AUDIO_EXPERIMENTAL */

OSWRAPPER_AUDIO_DEF void oswrapper_audio_rewind(OSWrapper_audio_spec* audio) {
    oswrapper_audio__internal_data_mac* internal_data = (oswrapper_audio__internal_data_mac*) audio->internal_data;
    ExtAudioFileSeek(internal_data->audio_file_ext, 0);
}

OSWRAPPER_AUDIO_DEF size_t oswrapper_audio_get_samples(OSWrapper_audio_spec* audio, short* buffer, size_t frames_to_do) {
    AudioBufferList buffer_list;
    UInt32 frames;
    oswrapper_audio__internal_data_mac* internal_data = (oswrapper_audio__internal_data_mac*) audio->internal_data;
    frames = frames_to_do;
    buffer_list.mNumberBuffers = 1;
    buffer_list.mBuffers[0].mNumberChannels = audio->channel_count;
    buffer_list.mBuffers[0].mDataByteSize = frames_to_do * ((audio->bits_per_channel / 8) * audio->channel_count);
    buffer_list.mBuffers[0].mData = buffer;
    ExtAudioFileRead(internal_data->audio_file_ext, &frames, &buffer_list);
    return frames;
}
/* End macOS AudioToolbox implementation */
#else
/* No audio loader implementation */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_init(void) {
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_uninit(void) {
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_free_context(OSWrapper_audio_spec* audio) {
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_memory(const unsigned char* data, size_t data_size, OSWrapper_audio_spec* audio) {
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

#ifndef OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_path(const char* path, OSWrapper_audio_spec* audio) {
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
/* Unstable-ish API */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_get_pos(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE* pos) {
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF void oswrapper_audio_seek(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE pos) {
    /* Nothing */
}
#endif /* OSWRAPPER_AUDIO_EXPERIMENTAL */

OSWRAPPER_AUDIO_DEF void oswrapper_audio_rewind(OSWrapper_audio_spec* audio) {
    /* Nothing */
}

OSWRAPPER_AUDIO_DEF size_t oswrapper_audio_get_samples(OSWrapper_audio_spec* audio, short* buffer, size_t frames_to_do) {
    return 0;
}
/* End no audio loader implementation */
#endif
#endif /* OSWRAPPER_AUDIO_IMPLEMENTATION */
#endif /* OSWRAPPER_INCLUDE_OSWRAPPER_AUDIO_H */

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
