/*
OSWrapper audio: Decode audio files with the built in OS audio decoders.

Usage:

OSWrapper audio has 2 ways of decoding audio: from a path, or from memory.
In both cases, you pass a pointer to an OSWrapper_audio_spec struct,
which acts as both a way to hint at the wanted audio format,
and to receive the actual audio format and decoding context.
Decoding to integer and floating point PCM is generally well supported.
More output formats may be added in the future.

Example:

// Initialise the library
if (!oswrapper_audio_init()) {
    // Error handling here
}

// Allocate an OSWrapper_audio_spec struct
OSWrapper_audio_spec* audio_spec = (OSWrapper_audio_spec*) malloc(sizeof(OSWrapper_audio_spec));

if (audio_spec == NULL) {
    // Error handling here
}

// Hint the desired format
audio_spec->sample_rate = SAMPLE_RATE;
audio_spec->channel_count = CHANNEL_COUNT;
audio_spec->bits_per_channel = BITS_PER_CHANNEL;
audio_spec->audio_type = AUDIO_FORMAT;

// Load audio from memory:
// if (oswrapper_audio_load_from_memory(data, data_size, audio_spec)) {
// Load audio from a path to a file:
if (oswrapper_audio_load_from_path(path, audio_spec)) {
    // audio_spec now contains the chosen format details (read-only, don't modify)
    // Calculate the size of one frame of audio
    size_t frame_size = (audio_spec->bits_per_channel / 8) * (audio_spec->channel_count);
    // Allocate a buffer capable of containing 512 frames of audio
    short* buffer = (short*) malloc(512 * frame_size);

    if (buffer == NULL) {
        // Error handling here
    }

    // Decode 512 frames of audio (or less, if the file is very small).
    // OSWrapper audio will try to always give you as many frames as you request.
    // 0 generally indicates EOF.
    size_t decoded = oswrapper_audio_get_samples(audio_spec, buffer, 512);

    // Do something with the frames of decoded audio

    // Free the buffer
    free(buffer);

    // Free the decoding context
    if (!oswrapper_audio_free_context(audio_spec)) {
        // Error handling here
    }
}

// Free the OSWrapper_audio_spec struct
free(audio_spec);

// Uninitialise the library
if (!oswrapper_audio_uninit()) {
    // Error handling
}

In a real scenario, you'd generally loop until there is no more audio to be decoded,
or handle setting up callbacks for decoding audio as needed.
See test_oswrapper_audio for an audio decoding program
which fully decodes a file to PCM data, and writes it to a new file.

Platform requirements:
- On macOS, link with AudioToolbox
- On Windows, call CoInitialize before using the library,
  and link with mfplat.lib, mfreadwrite.lib, and shlwapi.lib

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/oswrapper_audio.h
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

/* What format the decoded audio is in. */
typedef enum {
    OSWRAPPER_AUDIO_FORMAT_NOT_SET = 0,
    OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER,
    OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT
} OSWrapper_audio_type;

/* Endianness of the decoded audio. Most platforms use little-endian. */
typedef enum {
    OSWRAPPER_AUDIO_ENDIANNESS_USE_SYSTEM_DEFAULT = 0,
    OSWRAPPER_AUDIO_ENDIANNESS_LITTLE,
    OSWRAPPER_AUDIO_ENDIANNESS_BIG
} OSWrapper_audio_endianness_type;

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
    OSWrapper_audio_type audio_type;
    OSWrapper_audio_endianness_type endianness_type;
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
#ifndef OSWRAPPER_AUDIO_MEMCMP
#define OSWRAPPER_AUDIO_MEMCMP(ptr1, ptr2, amount) memcmp(ptr1, ptr2, amount)
#endif /* OSWRAPPER_AUDIO_MEMCMP */

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
#if !defined(OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL) && !defined(OSWRAPPER_AUDIO_NO_USE_AUDIOTOOLBOX_IMPL)
#define OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL
#endif /* !defined(OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL) && !defined(OSWRAPPER_AUDIO_NO_USE_AUDIOTOOLBOX_IMPL) */
#endif /* defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4 */
#endif /* __APPLE__ */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#if !defined(OSWRAPPER_AUDIO_USE_WIN_MF_IMPL) && !defined(OSWRAPPER_AUDIO_NO_USE_WIN_MF_IMPL)
#define OSWRAPPER_AUDIO_USE_WIN_MF_IMPL
#endif /* !defined(OSWRAPPER_AUDIO_USE_WIN_MF_IMPL) && !defined(OSWRAPPER_AUDIO_NO_USE_WIN_MF_IMPL) */
#endif

#ifdef OSWRAPPER_AUDIO_USE_AUDIOTOOLBOX_IMPL
/* Start macOS AudioToolbox implementation */
#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/ExtendedAudioFile.h>

#include <AvailabilityMacros.h>

#if !defined(MAC_OS_X_VERSION_10_6) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
#define kAudioFileReadPermission 0x01
#endif

#if !defined(MAC_OS_X_VERSION_10_13) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_13
#define kAudioFormatFLAC 'flac'
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

            /* Use hinted output format */
            if (audio->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
                output_format.mFormatFlags = kLinearPCMFormatFlagIsFloat;
            } else if (audio->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER) {
                output_format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
            } else if (input_file_format.mFormatID == kAudioFormatLinearPCM && input_file_format.mFormatFlags & kLinearPCMFormatFlagIsFloat) {
                output_format.mFormatFlags = kLinearPCMFormatFlagIsFloat;
            } else {
                output_format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
            }

            output_format.mFormatFlags |= kLinearPCMFormatFlagIsPacked;

            /* Use hinted endianness */
            if (audio->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG) {
                output_format.mFormatFlags |= kAudioFormatFlagIsBigEndian;
            }

#if defined(__ppc64__) || defined(__ppc__)
            else if (audio->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_USE_SYSTEM_DEFAULT) {
                output_format.mFormatFlags |= kAudioFormatFlagIsBigEndian;
            }

#endif
            /* Use hinted sample rate */
            output_format.mSampleRate = audio->sample_rate == 0 ? input_file_format.mSampleRate : audio->sample_rate;

            /* Sanity check */
            if (output_format.mSampleRate == 0) {
                output_format.mSampleRate = 44100;
            }

            /* Use hinted bits per channel */
            output_format.mBitsPerChannel = audio->bits_per_channel == 0 ? input_file_format.mBitsPerChannel : audio->bits_per_channel;

            /* Get bits per channel for ALAC and FLAC files */
            if ((output_format.mBitsPerChannel == 0) && (input_file_format.mFormatID == kAudioFormatAppleLossless || input_file_format.mFormatID == kAudioFormatFLAC)) {
                switch (input_file_format.mFormatFlags) {
                case kAppleLosslessFormatFlag_16BitSourceData:
                    output_format.mBitsPerChannel = 16;
                    break;

                case kAppleLosslessFormatFlag_20BitSourceData:
                    output_format.mBitsPerChannel = 20;
                    break;

                case kAppleLosslessFormatFlag_24BitSourceData:
                    output_format.mBitsPerChannel = 24;
                    break;

                case kAppleLosslessFormatFlag_32BitSourceData:
                    output_format.mBitsPerChannel = 32;
                    break;

                default:
                    break;
                }
            }

            /* Sanity check */
            if (output_format.mBitsPerChannel == 0) {
                if (output_format.mFormatFlags & kLinearPCMFormatFlagIsFloat) {
                    output_format.mBitsPerChannel = 32;
                } else {
                    output_format.mBitsPerChannel = 16;
                }
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
                oswrapper_audio__internal_data_mac* internal_data = NULL;

                if (input_file_format.mChannelsPerFrame == 1 && output_format.mChannelsPerFrame == 2) {
                    /* Try to set up appropriate channel mappings */
                    AudioConverterRef converter = NULL;
                    property_size = sizeof(AudioConverterRef);

                    if (!ExtAudioFileGetProperty(audio_file_ext, kExtAudioFileProperty_AudioConverter, &property_size, &converter) && converter != NULL) {
                        /* Output from input mono channel to both stereo channels */
                        SInt32 channel_map[2] = {0, 0};
                        /* Failure is mostly harmless */
                        AudioConverterSetProperty(converter, kAudioConverterChannelMap, sizeof(channel_map), channel_map);
                        CFArrayRef converter_config = NULL;
                        ExtAudioFileSetProperty(audio_file_ext, kExtAudioFileProperty_ConverterConfig, sizeof(CFArrayRef), &converter_config);
                    }
                }

                internal_data = (oswrapper_audio__internal_data_mac*) OSWRAPPER_AUDIO_MALLOC(sizeof(oswrapper_audio__internal_data_mac));

                if (internal_data != NULL) {
                    audio->sample_rate = output_format.mSampleRate;
                    audio->bits_per_channel = output_format.mBitsPerChannel;
                    audio->channel_count = output_format.mChannelsPerFrame;
                    audio->audio_type = (output_format.mFormatFlags & kLinearPCMFormatFlagIsFloat) ? OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT : OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER;
                    audio->endianness_type = (output_format.mFormatFlags & kAudioFormatFlagIsBigEndian) ? OSWRAPPER_AUDIO_ENDIANNESS_BIG : OSWRAPPER_AUDIO_ENDIANNESS_LITTLE;
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
#elif defined(OSWRAPPER_AUDIO_USE_WIN_MF_IMPL)
/* Start Win32 MF implementation */
#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cguid.h>
#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <shlwapi.h>
#include <stdio.h>

/* Using CINTERFACE breaks some headers, so we have to define these ourselves */
#if defined(__cplusplus) && !defined(CINTERFACE)
#define IMFAttributes_GetGUID(attributes, ...) attributes->GetGUID(__VA_ARGS__)
#define IMFByteStream_Release(byte_stream) byte_stream->Release()
#define IMFMediaBuffer_Lock(media_buffer, ...) media_buffer->Lock(__VA_ARGS__)
#define IMFMediaBuffer_Release(media_buffer) media_buffer->Release()
#define IMFMediaBuffer_Unlock(media_buffer) media_buffer->Unlock()
#define IMFMediaType_GetUINT32(media_type, ...) media_type->GetUINT32(__VA_ARGS__)
#define IMFMediaType_Release(media_type) media_type->Release()
#define IMFMediaType_SetGUID(media_type, ...) media_type->SetGUID(__VA_ARGS__)
#define IMFMediaType_SetUINT32(media_type, ...) media_type->SetUINT32(__VA_ARGS__)
#define IMFSample_ConvertToContiguousBuffer(sample, ...) sample->ConvertToContiguousBuffer(__VA_ARGS__)
#define IMFSample_Release(sample) sample->Release()
#define IMFSourceReader_GetNativeMediaType(source_reader, ...) source_reader->GetNativeMediaType(__VA_ARGS__)
#define IMFSourceReader_ReadSample(source_reader, ...) source_reader->ReadSample(__VA_ARGS__)
#define IMFSourceReader_Release(source_reader) source_reader->Release()
#define IMFSourceReader_SetCurrentMediaType(source_reader, ...) source_reader->SetCurrentMediaType(__VA_ARGS__)
#define IMFSourceReader_SetCurrentPosition(source_reader, ...) source_reader->SetCurrentPosition(__VA_ARGS__)
#define IMFSourceReader_SetStreamSelection(source_reader, ...) source_reader->SetStreamSelection(__VA_ARGS__)
#define IStream_Release(istream) istream->Release()
#endif

/* TODO Ugly hack */
#ifndef OSWRAPPER_AUDIO_PATH_MAX
#define OSWRAPPER_AUDIO_PATH_MAX MAX_PATH
#endif

#ifdef OSWRAPPER_AUDIO_MANAGE_COINIT
#include <objbase.h>

#ifndef OSWRAPPER_AUDIO_COINIT_VALUE
#define OSWRAPPER_AUDIO_COINIT_VALUE COINIT_MULTITHREADED
#endif
#endif

/* The startup flags forMFStartup */
#ifndef OSWRAPPER_AUDIO__MF_STARTUP_VAL
#define OSWRAPPER_AUDIO__MF_STARTUP_VAL MFSTARTUP_LITE
#endif

typedef struct oswrapper_audio__internal_data_win {
    IMFSourceReader* reader;
    IMFByteStream* byte_stream;
    IStream* memory_stream;
    short* internal_buffer;
    /* Buffer size */
    size_t internal_buffer_size;
    /* Remaining samples */
    size_t internal_buffer_remaining;
    /* Current position */
    size_t internal_buffer_pos;
    /* Has the reader thrown an error? */
    OSWRAPPER_AUDIO_RESULT_TYPE no_reader_error;
} oswrapper_audio__internal_data_win;

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_init(void) {
#ifdef OSWRAPPER_AUDIO_MANAGE_COINIT
    HRESULT result = CoInitializeEx(NULL, OSWRAPPER_AUDIO_COINIT_VALUE);

    if (SUCCEEDED(result)) {
        result = MFStartup(MF_VERSION, OSWRAPPER_AUDIO__MF_STARTUP_VAL);

        if (SUCCEEDED(result)) {
            return OSWRAPPER_AUDIO_RESULT_SUCCESS;
        }

        CoUninitialize();
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
#else
    return SUCCEEDED(MFStartup(MF_VERSION, OSWRAPPER_AUDIO__MF_STARTUP_VAL)) ? OSWRAPPER_AUDIO_RESULT_SUCCESS : OSWRAPPER_AUDIO_RESULT_FAILURE;
#endif
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_uninit(void) {
#ifdef OSWRAPPER_AUDIO_MANAGE_COINIT

    if (FAILED(MFShutdown())) {
        return OSWRAPPER_AUDIO_RESULT_FAILURE;
    }

    CoUninitialize();
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
#else
    return SUCCEEDED(MFShutdown()) ? OSWRAPPER_AUDIO_RESULT_SUCCESS : OSWRAPPER_AUDIO_RESULT_FAILURE;
#endif
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_free_context(OSWrapper_audio_spec* audio) {
    oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;
    IMFSourceReader_Release(internal_data->reader);

    /* Only expected for in-memory decoding */
    if (internal_data->byte_stream != NULL) {
        IMFByteStream_Release(internal_data->byte_stream);
    }

    /* Only expected for in-memory decoding */
    if (internal_data->memory_stream != NULL) {
        IStream_Release(internal_data->memory_stream);
    }

    if (internal_data->internal_buffer != NULL) {
        OSWRAPPER_AUDIO_FREE(internal_data->internal_buffer);
    }

    OSWRAPPER_AUDIO_FREE(audio->internal_data);
    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

#define OSWRAPPER_AUDIO__END_FAIL_FALSE(cond) if(!cond) { return_val = OSWRAPPER_AUDIO_RESULT_FAILURE; goto cleanup; }
#define OSWRAPPER_AUDIO__END_FAIL(hres) OSWRAPPER_AUDIO__END_FAIL_FALSE(SUCCEEDED(hres))

static OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio__configure_stream(IMFSourceReader* reader, OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_RESULT_TYPE initial_configure) {
    OSWRAPPER_AUDIO_RESULT_TYPE return_val;
    HRESULT result;
    UINT32 sample_rate, channel_count, bits_per_channel;
    GUID format_type;
    IMFMediaType* media_type;
    oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

    if ((initial_configure != OSWRAPPER_AUDIO_RESULT_SUCCESS) && (internal_data != NULL) && (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_FAILURE)) {
        /* IMFSourceReader methods can no longer be called */
        return OSWRAPPER_AUDIO_RESULT_FAILURE;
    }

    media_type = NULL;
    result = S_OK;
    return_val = OSWRAPPER_AUDIO_RESULT_SUCCESS;
    channel_count = audio->channel_count;
    sample_rate = audio->sample_rate;
    bits_per_channel = audio->bits_per_channel;

    if (audio->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
        format_type = MFAudioFormat_Float;
    } else if (audio->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER) {
        format_type = MFAudioFormat_PCM;
    } else {
        /* Fallback */
        format_type = MFAudioFormat_PCM;
    }

    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetStreamSelection(reader, (DWORD) MF_SOURCE_READER_ALL_STREAMS, FALSE));
    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetStreamSelection(reader, (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));

    /* Get / set the format */
    if ((sample_rate == 0) || (channel_count == 0) || (bits_per_channel == 0) || (audio->audio_type == OSWRAPPER_AUDIO_FORMAT_NOT_SET)) {
        OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_GetNativeMediaType(reader, (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &media_type));

        if (channel_count == 0) {
#ifdef __cplusplus
            IMFMediaType_GetUINT32(media_type, MF_MT_AUDIO_NUM_CHANNELS, &channel_count);
#else
            IMFMediaType_GetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, &channel_count);
#endif

            /* Sanity check */
            if (channel_count == 0) {
                channel_count = 2;
            }

            audio->channel_count = channel_count;
        }

        if (sample_rate == 0) {
#ifdef __cplusplus
            IMFMediaType_GetUINT32(media_type, MF_MT_AUDIO_SAMPLES_PER_SECOND, &sample_rate);
#else
            IMFMediaType_GetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, &sample_rate);
#endif

            /* Sanity check */
            if (sample_rate == 0) {
                sample_rate = 44100;
            }

            audio->sample_rate = sample_rate;
        }

        if (audio->audio_type == OSWRAPPER_AUDIO_FORMAT_NOT_SET) {
#ifdef __cplusplus
            IMFAttributes_GetGUID(media_type, MF_MT_SUBTYPE, &format_type);
#else
            IMFAttributes_GetGUID(media_type, &MF_MT_SUBTYPE, &format_type);
#endif

            /* Sanity check */
            if (!OSWRAPPER_AUDIO_MEMCMP(&format_type, &MFAudioFormat_PCM, sizeof(GUID))) {
                audio->audio_type = OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER;
            } else if (!OSWRAPPER_AUDIO_MEMCMP(&format_type, &MFAudioFormat_Float, sizeof(GUID))) {
                audio->audio_type = OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT;
            } else {
                /* Fallback */
                format_type = MFAudioFormat_PCM;
                audio->audio_type = OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER;
            }
        }

        if (bits_per_channel == 0) {
#ifdef __cplusplus
            IMFMediaType_GetUINT32(media_type, MF_MT_AUDIO_BITS_PER_SAMPLE, &bits_per_channel);
#else
            IMFMediaType_GetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, &bits_per_channel);
#endif

            /* Sanity check */
            if (bits_per_channel == 0) {
                if (audio->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
                    bits_per_channel = 32;
                } else {
                    bits_per_channel = 16;
                }
            }

            audio->bits_per_channel = bits_per_channel;
        }

        IMFMediaType_Release(media_type);
    }

    OSWRAPPER_AUDIO__END_FAIL(MFCreateMediaType(&media_type));
#ifdef __cplusplus
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetGUID(media_type, MF_MT_MAJOR_TYPE, MFMediaType_Audio));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetGUID(media_type, MF_MT_SUBTYPE, format_type));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_BITS_PER_SAMPLE, audio->bits_per_channel));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_SAMPLES_PER_SECOND, audio->sample_rate));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_NUM_CHANNELS, audio->channel_count));
#else
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetGUID(media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetGUID(media_type, &MF_MT_SUBTYPE, &format_type));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio->bits_per_channel));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio->sample_rate));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio->channel_count));
#endif
    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetCurrentMediaType(reader, (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, media_type));
    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetStreamSelection(reader, (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));
cleanup:

    if (media_type != NULL) {
        IMFMediaType_Release(media_type);
    }

    return return_val;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio__load_from_reader(IMFSourceReader* reader, IMFByteStream* byte_stream, IStream* memory_stream, OSWrapper_audio_spec* audio) {
    if (oswrapper_audio__configure_stream(reader, audio, OSWRAPPER_AUDIO_RESULT_SUCCESS)) {
        oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) OSWRAPPER_AUDIO_MALLOC(sizeof(oswrapper_audio__internal_data_win));

        if (internal_data != NULL) {
            audio->endianness_type = OSWRAPPER_AUDIO_ENDIANNESS_LITTLE;
            audio->internal_data = (void*) internal_data;
            internal_data->reader = reader;
            internal_data->byte_stream = byte_stream;
            internal_data->memory_stream = memory_stream;
            internal_data->internal_buffer = NULL;
            internal_data->internal_buffer_pos = 0;
            internal_data->internal_buffer_remaining = 0;
            internal_data->internal_buffer_size = 0;
            internal_data->no_reader_error = OSWRAPPER_AUDIO_RESULT_SUCCESS;
            return OSWRAPPER_AUDIO_RESULT_SUCCESS;
        }
    }

    IMFSourceReader_Release(reader);

    /* Only expected for in-memory decoding */
    if (byte_stream != NULL) {
        IMFByteStream_Release(byte_stream);
    }

    /* Only expected for in-memory decoding */
    if (memory_stream != NULL) {
        IStream_Release(memory_stream);
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_memory(const unsigned char* data, size_t data_size, OSWrapper_audio_spec* audio) {
    IStream* memory_stream = SHCreateMemStream(data, (UINT) data_size);

    if (memory_stream != NULL) {
        HRESULT result;
        IMFByteStream* byte_stream = NULL;
        result = MFCreateMFByteStreamOnStream(memory_stream, &byte_stream);

        if (SUCCEEDED(result)) {
            IMFSourceReader* reader = NULL;
            result = MFCreateSourceReaderFromByteStream(byte_stream, NULL, &reader);

            if (SUCCEEDED(result)) {
                return oswrapper_audio__load_from_reader(reader, byte_stream, memory_stream, audio);
            }

            IMFByteStream_Release(byte_stream);
        }

        IStream_Release(memory_stream);
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

#ifndef OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_path(const char* path, OSWrapper_audio_spec* audio) {
    HRESULT result;
    /* TODO Ugly hack */
    wchar_t path_buffer[OSWRAPPER_AUDIO_PATH_MAX];
    result = MultiByteToWideChar(CP_UTF8, 0, path, -1, path_buffer, OSWRAPPER_AUDIO_PATH_MAX);

    if (SUCCEEDED(result)) {
        IMFSourceReader* reader = NULL;
        result = MFCreateSourceReaderFromURL(path_buffer, NULL, &reader);

        if (SUCCEEDED(result)) {
            return oswrapper_audio__load_from_reader(reader, NULL, NULL, audio);
        }
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
/* Unstable-ish API */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_get_pos(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE* pos) {
    /* TODO Unimplemented
      oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

      if (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_FAILURE) {
          / * IMFSourceReader methods can no longer be called * /
          return OSWRAPPER_AUDIO_RESULT_FAILURE;
      }
      */
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF void oswrapper_audio_seek(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE pos) {
    oswrapper_audio__internal_data_win* internal_data;
    HRESULT result;
    PROPVARIANT pos_propvariant = { 0 };
    internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

    if (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_FAILURE) {
        /* IMFSourceReader methods can no longer be called */
        return;
    }

    result = S_OK;
    pos_propvariant.vt = VT_I8;
    pos_propvariant.hVal.QuadPart = pos;
#ifdef __cplusplus
    IMFSourceReader_SetCurrentPosition(internal_data->reader, GUID_NULL, pos_propvariant);
#else
    IMFSourceReader_SetCurrentPosition(internal_data->reader, &GUID_NULL, &pos_propvariant);
#endif
    PropVariantClear(&pos_propvariant);
    /* Reset buffer positions */
    internal_data->internal_buffer_remaining = 0;
    internal_data->internal_buffer_pos = 0;
}
#endif /* OSWRAPPER_AUDIO_EXPERIMENTAL */

OSWRAPPER_AUDIO_DEF void oswrapper_audio_rewind(OSWrapper_audio_spec* audio) {
    oswrapper_audio__internal_data_win* internal_data;
    HRESULT result;
    PROPVARIANT pos_propvariant = { 0 };
    internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

    if (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_FAILURE) {
        /* IMFSourceReader methods can no longer be called */
        return;
    }

    result = S_OK;
    pos_propvariant.vt = VT_I8;
    pos_propvariant.hVal.QuadPart = 0;
#ifdef __cplusplus
    IMFSourceReader_SetCurrentPosition(internal_data->reader, GUID_NULL, pos_propvariant);
#else
    IMFSourceReader_SetCurrentPosition(internal_data->reader, &GUID_NULL, &pos_propvariant);
#endif
    PropVariantClear(&pos_propvariant);
    /* Reset buffer positions */
    internal_data->internal_buffer_remaining = 0;
    internal_data->internal_buffer_pos = 0;
}

OSWRAPPER_AUDIO_DEF size_t oswrapper_audio_get_samples(OSWrapper_audio_spec* audio, short* buffer, size_t frames_to_do) {
    size_t frame_size;
    size_t frames_done;
    oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;
    frame_size = (audio->bits_per_channel / 8) * audio->channel_count;
    frames_to_do = frames_to_do * frame_size / sizeof(short);
    frames_done = 0;

    /* Copy any previous excess frames from the internal buffer */
    if (internal_data->internal_buffer_remaining > 0) {
        size_t copied_sample_data_size = internal_data->internal_buffer_remaining < frames_to_do ? internal_data->internal_buffer_remaining : frames_to_do;
        OSWRAPPER_AUDIO_MEMCPY(buffer, internal_data->internal_buffer + internal_data->internal_buffer_pos, copied_sample_data_size * (sizeof internal_data->internal_buffer[0]));
        internal_data->internal_buffer_remaining -= copied_sample_data_size;
        internal_data->internal_buffer_pos += copied_sample_data_size;
        frames_done = copied_sample_data_size;
    }

    /* Get new samples */
    while (frames_to_do > frames_done) {
        size_t new_target_frames = 0;

        /* Check if IMFSourceReader methods can still be called */
        if (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_SUCCESS) {
            IMFSample* sample;
            DWORD flags;
            HRESULT result;
            sample = NULL;
            result = IMFSourceReader_ReadSample(internal_data->reader, (DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &flags, NULL, &sample);

            if (SUCCEEDED(result)) {
                /* TODO handle more flags */
                if (flags & MF_SOURCE_READERF_ERROR) {
                    /* IMFSourceReader methods can no longer be called */
                    internal_data->no_reader_error = OSWRAPPER_AUDIO_RESULT_FAILURE;
                } else if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) || (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) || (sample == NULL)) {
                    /* TODO handle these more gracefully */
                } else {
                    IMFMediaBuffer* media_buffer;
                    result = IMFSample_ConvertToContiguousBuffer(sample, &media_buffer);

                    if (SUCCEEDED(result)) {
                        DWORD current_length;
                        BYTE* sample_audio_data = NULL;
                        result = IMFMediaBuffer_Lock(media_buffer, &sample_audio_data, NULL, &current_length);

                        if (SUCCEEDED(result)) {
                            size_t new_target_size;
                            new_target_frames = current_length / sizeof(short);
                            new_target_size = frames_done + new_target_frames;

                            /* If the size of the decoded sample would exceed the remaining buffer size,
                               store the excess frames in the internal buffer */
                            if (new_target_size > frames_to_do) {
                                size_t remaining_sample_data_size = new_target_size - frames_to_do;
                                /* Prevent copying more data to the output buffer than requested */
                                new_target_frames -= remaining_sample_data_size;
                                current_length = (DWORD) new_target_frames * sizeof(short);

                                /* Is the internal buffer large enough to store the excess frames? */
                                if (internal_data->internal_buffer_size < remaining_sample_data_size) {
                                    /* Try to allocate enough memory to store the excess frames */
                                    short* realloc_buffer = (short*) OSWRAPPER_AUDIO_MALLOC(remaining_sample_data_size * sizeof(short));

                                    if (realloc_buffer != NULL) {
                                        /* Free old buffer */
                                        OSWRAPPER_AUDIO_FREE(internal_data->internal_buffer);
                                        /* Replace old buffer with new buffer */
                                        internal_data->internal_buffer = realloc_buffer;
                                        internal_data->internal_buffer_size = remaining_sample_data_size;
                                    } else {
                                        /* If we can't allocate more memory, some excess frames will be lost.
                                           This is unlikely, and mostly harmless. */
                                        remaining_sample_data_size = internal_data->internal_buffer_size;
                                    }
                                }

                                /* Copy as many excess frames as we have space for */
                                internal_data->internal_buffer_remaining = remaining_sample_data_size;
                                internal_data->internal_buffer_pos = 0;

                                if (remaining_sample_data_size > 0) {
                                    OSWRAPPER_AUDIO_MEMCPY((BYTE*)(internal_data->internal_buffer), sample_audio_data + current_length, remaining_sample_data_size * sizeof(short));
                                }
                            }

                            /* Copy decoded sample data to buffer, minus excess frames */
                            if (new_target_frames > 0) {
                                OSWRAPPER_AUDIO_MEMCPY((BYTE*)(buffer + frames_done), sample_audio_data, current_length);
                            }

                            /* result = */ IMFMediaBuffer_Unlock(media_buffer);
                            /* TODO I'm not sure there's any way to handle this?
                            if (FAILED(result)) {
                                ...some code?
                            }*/
                        }

                        IMFMediaBuffer_Release(media_buffer);
                    }
                }
            }

            /* May be non-null even on failure */
            if (sample != NULL) {
                IMFSample_Release(sample);
            }
        }

        if (new_target_frames == 0) {
            /* Break the loop */
            break;
        } else {
            frames_done += new_target_frames;
        }
    }

    return frames_done * sizeof(short) / frame_size;
}
/* End Win32 MF implementation */
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
