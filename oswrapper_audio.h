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
#ifndef OSWRAPPER_AUDIO_CALLOC
#define OSWRAPPER_AUDIO_CALLOC(size, x) calloc(size, x)
#endif /* OSWRAPPER_AUDIO_CALLOC */
#ifndef OSWRAPPER_AUDIO_REALLOC
#define OSWRAPPER_AUDIO_REALLOC(buf, size) realloc(buf, size)
#endif /* OSWRAPPER_AUDIO_REALLOC */
#ifndef OSWRAPPER_AUDIO_FREE
#define OSWRAPPER_AUDIO_FREE(x) free(x)
#endif /* OSWRAPPER_AUDIO_FREE */
#ifndef OSWRAPPER_AUDIO_MEMCPY
#define OSWRAPPER_AUDIO_MEMCPY(x, y, amount) memcpy(x, y, amount)
#endif /* OSWRAPPER_AUDIO_MEMCPY */
#ifndef OSWRAPPER_AUDIO_MEMMOVE
#define OSWRAPPER_AUDIO_MEMMOVE(x, y, amount) memmove(x, y, amount)
#endif /* OSWRAPPER_AUDIO_MEMMOVE */

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
#elif defined(OSWRAPPER_AUDIO_USE_WIN_MF_IMPL)
/* WIP: This code half-works, and by that, I mean it gives you half the samples you want */
/* Start Win32 MF implementation */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <initguid.h>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <shlwapi.h>

/* TODO linking bodge */
extern const IID GUID_NULL;

/* TODO Ugly hack */
#ifndef OSWRAPPER_AUDIO_PATH_MAX
#define OSWRAPPER_AUDIO_PATH_MAX MAX_PATH
#endif

#ifndef OSWRAPPER_AUDIO__INTERNAL_BUFFER_SIZE
/* TODO The program seems to crash unless realloc is called later??? 0x20000 might be a good size */
#define OSWRAPPER_AUDIO__INTERNAL_BUFFER_SIZE 0x40
#endif

typedef struct oswrapper_audio__internal_data_win {
    IMFSourceReader* reader;
    short* internal_buffer;
    /* Real size */
    size_t internal_buffer_size;
    /* Remaining samples TODO optimise */
    size_t internal_buffer_remaining;
    /* Current position */
    size_t internal_buffer_pos;
    /* Has the reader thrown an error? */
    OSWRAPPER_AUDIO_RESULT_TYPE no_reader_error;
} oswrapper_audio__internal_data_win;

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_init(void) {
    return SUCCEEDED(MFStartup(MF_VERSION, MFSTARTUP_LITE)) ? OSWRAPPER_AUDIO_RESULT_SUCCESS : OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_uninit(void) {
    return SUCCEEDED(MFShutdown()) ? OSWRAPPER_AUDIO_RESULT_SUCCESS : OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_free_context(OSWrapper_audio_spec* audio) {
    oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;
    IMFSourceReader_Release(internal_data->reader);

    if (internal_data->internal_buffer != NULL) {
        OSWRAPPER_AUDIO_FREE(internal_data->internal_buffer);
    }

    return OSWRAPPER_AUDIO_RESULT_SUCCESS;
}

#define OSWRAPPER_AUDIO__END_FAIL_FALSE(cond) if(!cond) { return_val = OSWRAPPER_AUDIO_RESULT_FAILURE; goto cleanup; }
#define OSWRAPPER_AUDIO__END_FAIL(hres) OSWRAPPER_AUDIO__END_FAIL_FALSE(SUCCEEDED(hres))

static OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio__configure_stream(IMFSourceReader* reader, OSWrapper_audio_spec* audio) {
    oswrapper_audio__internal_data_win* internal_data;
    OSWRAPPER_AUDIO_RESULT_TYPE return_val;
    HRESULT result;
    UINT32 sample_rate, channel_count, bits_per_channel;
    internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

    if ((internal_data != NULL) && (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_FAILURE)) {
        /* IMFSourceReader methods can no longer be called */
        return OSWRAPPER_AUDIO_RESULT_FAILURE;
    }

    IMFMediaType* media_type = NULL;
    result = S_OK;
    return_val = OSWRAPPER_AUDIO_RESULT_SUCCESS;
    channel_count = audio->channel_count;
    sample_rate = audio->sample_rate;
    bits_per_channel = audio->bits_per_channel;
    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetStreamSelection(reader, MF_SOURCE_READER_ALL_STREAMS, FALSE));
    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetStreamSelection(reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));

    /* Get / set the format */
    if ((sample_rate == 0) || (channel_count == 0) || (bits_per_channel == 0)) {
        OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_GetNativeMediaType(reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &media_type));

        if (channel_count == 0) {
            IMFMediaType_GetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, &channel_count);

            /* Sanity check */
            if (channel_count == 0) {
                channel_count = 2;
            }

            audio->channel_count = channel_count;
        }

        if (sample_rate == 0) {
            IMFMediaType_GetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, &sample_rate);

            /* Sanity check */
            if (sample_rate == 0) {
                sample_rate = 44100;
            }

            audio->sample_rate = sample_rate;
        }

        if (bits_per_channel == 0) {
            IMFMediaType_GetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, &bits_per_channel);

            /* Sanity check */
            if (bits_per_channel == 0) {
                bits_per_channel = 16;
            }

            audio->bits_per_channel = bits_per_channel;
        }

        IMFMediaType_Release(media_type);
    }

    OSWRAPPER_AUDIO__END_FAIL(MFCreateMediaType(&media_type));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetGUID(media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetGUID(media_type, &MF_MT_SUBTYPE, &MFAudioFormat_PCM));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio->bits_per_channel));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio->sample_rate));
    OSWRAPPER_AUDIO__END_FAIL(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio->channel_count));
    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetCurrentMediaType(reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, media_type));
    OSWRAPPER_AUDIO__END_FAIL(IMFSourceReader_SetStreamSelection(reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));
cleanup:

    if (media_type != NULL) {
        IMFMediaType_Release(media_type);
    }

    return return_val;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio__load_from_reader(IMFSourceReader* reader, OSWrapper_audio_spec* audio) {
    if (oswrapper_audio__configure_stream(reader, audio)) {
        oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) OSWRAPPER_AUDIO_MALLOC(sizeof(oswrapper_audio__internal_data_win));

        if (internal_data != NULL) {
            audio->internal_data = (void*) internal_data;
            internal_data->reader = reader;
            /* TODO rework */
            internal_data->internal_buffer = (short*) OSWRAPPER_AUDIO_CALLOC(sizeof(short), OSWRAPPER_AUDIO__INTERNAL_BUFFER_SIZE);
            internal_data->internal_buffer_pos = 0;
            internal_data->internal_buffer_remaining = 0;
            internal_data->internal_buffer_size = internal_data->internal_buffer == NULL ? 0 : OSWRAPPER_AUDIO__INTERNAL_BUFFER_SIZE;
            internal_data->no_reader_error = OSWRAPPER_AUDIO_RESULT_SUCCESS;
            return OSWRAPPER_AUDIO_RESULT_SUCCESS;
        }
    }

    IMFSourceReader_Release(reader);
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_memory(const unsigned char* data, size_t data_size, OSWrapper_audio_spec* audio) {
    IStream* memory_stream = SHCreateMemStream(data, data_size);

    if (memory_stream != NULL) {
        HRESULT result;
        IMFByteStream* byte_stream = NULL;
        result = MFCreateMFByteStreamOnStream(memory_stream, &byte_stream);

        if (SUCCEEDED(result)) {
            IMFSourceReader* reader = NULL;
            result = MFCreateSourceReaderFromByteStream(byte_stream, NULL, &reader);

            if (SUCCEEDED(result)) {
                return oswrapper_audio__load_from_reader(reader, audio);
            }
        }
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

#ifndef OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_load_from_path(const char* path, OSWrapper_audio_spec* audio) {
    /* TODO Ugly hack */
    wchar_t path_buffer[OSWRAPPER_AUDIO_PATH_MAX];
    HRESULT result = MultiByteToWideChar(CP_UTF8, 0, path, -1, path_buffer, OSWRAPPER_AUDIO_PATH_MAX);

    if (SUCCEEDED(result)) {
        IMFSourceReader* reader = NULL;
        result = MFCreateSourceReaderFromURL(path_buffer, NULL, &reader);

        if (SUCCEEDED(result)) {
            return oswrapper_audio__load_from_reader(reader, audio);
        }
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
/* Unstable-ish API */
OSWRAPPER_AUDIO_DEF OSWRAPPER_AUDIO_RESULT_TYPE oswrapper_audio_get_pos(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE* pos) {
    oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

    if (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_FAILURE) {
        /* IMFSourceReader methods can no longer be called */
        return OSWRAPPER_AUDIO_RESULT_FAILURE;
    }

    /* TODO
    IMFSourceReader* reader = internal_data->reader; */
    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_DEF void oswrapper_audio_seek(OSWrapper_audio_spec* audio, OSWRAPPER_AUDIO_SEEK_TYPE pos) {
    oswrapper_audio__internal_data_win* internal_data;
    HRESULT result;
    PROPVARIANT pos_propvariant = { 0 };
    internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

    if (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_FAILURE) {
        /* IMFSourceReader methods can no longer be called */
        return OSWRAPPER_AUDIO_RESULT_FAILURE;
    }

    /* TODO reset buffers */
    result = S_OK;
    pos_propvariant.vt = VT_I8;
    pos_propvariant.hVal.QuadPart = pos;
    IMFSourceReader_SetCurrentPosition(internal_data->reader, &GUID_NULL, &pos_propvariant);
    PropVariantClear(&pos_propvariant);
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

    /* TODO reset buffers */
    result = S_OK;
    pos_propvariant.vt = VT_I8;
    pos_propvariant.hVal.QuadPart = 0;
    IMFSourceReader_SetCurrentPosition(internal_data->reader, &GUID_NULL, &pos_propvariant);
    PropVariantClear(&pos_propvariant);
}

/* TODO This code isn't optimised. If you're brave enough, you can probably speed it up and save memory. */
static void oswrapper_audio__get_new_samples(OSWrapper_audio_spec* audio, size_t frames_to_do) {
    oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;

    /* Move any remaining samples to the start of the buffer, if they're not at the start of the buffer */
    if (internal_data->internal_buffer_remaining > 0 && (internal_data->internal_buffer_pos != 0)) {
        OSWRAPPER_AUDIO_MEMMOVE(internal_data->internal_buffer, internal_data->internal_buffer + internal_data->internal_buffer_pos, internal_data->internal_buffer_remaining * (sizeof internal_data->internal_buffer[0]));
    }

    /* Reset the buffer position to 0 */
    internal_data->internal_buffer_pos = 0;

    /* Get new samples */
    while (frames_to_do > internal_data->internal_buffer_remaining) {
        size_t new_target_frames = 0;

        if (internal_data->no_reader_error == OSWRAPPER_AUDIO_RESULT_SUCCESS) {
            IMFSample* sample;
            DWORD flags;
            HRESULT result;
            sample = NULL;
            result = IMFSourceReader_ReadSample(internal_data->reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &flags, NULL, &sample);

            if (SUCCEEDED(result)) {
                /* TODO handle more flags */
                if (flags & MF_SOURCE_READERF_ERROR) {
                    /* IMFSourceReader methods can no longer be called */
                    internal_data->no_reader_error = OSWRAPPER_AUDIO_RESULT_FAILURE;
                } else if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) || (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) || (sample == NULL)) {
                    /* TODO handle these better */
                } else {
                    IMFMediaBuffer* media_buffer;
                    result = IMFSample_ConvertToContiguousBuffer(sample, &media_buffer);

                    if (SUCCEEDED(result)) {
                        DWORD current_length;
                        BYTE* sample_audio_data = NULL;
                        result = IMFMediaBuffer_Lock(media_buffer, &sample_audio_data, NULL, &current_length);

                        if (SUCCEEDED(result)) {
                            new_target_frames = current_length;
                            size_t new_target_size = internal_data->internal_buffer_remaining + new_target_frames;

                            if (new_target_size > internal_data->internal_buffer_size) {
                                short* realloc_buffer = (short*) OSWRAPPER_AUDIO_MALLOC(new_target_size * sizeof(short));

                                if (realloc_buffer == NULL) {
                                    /* Couldn't alloc any more memory, just work with what we have */
                                    new_target_frames = internal_data->internal_buffer_size - internal_data->internal_buffer_remaining;
                                    frames_to_do = internal_data->internal_buffer_remaining + new_target_frames;
                                } else {
                                    OSWRAPPER_AUDIO_FREE(internal_data->internal_buffer);
                                    internal_data->internal_buffer = realloc_buffer;
                                    internal_data->internal_buffer_size = new_target_size;
                                }
                            }

                            if (new_target_frames > 0) {
                                OSWRAPPER_AUDIO_MEMCPY((BYTE*)(internal_data->internal_buffer + internal_data->internal_buffer_remaining), sample_audio_data, new_target_frames);
                            }

                            result = IMFMediaBuffer_Unlock(media_buffer);

                            if (FAILED(result)) {
                                /* TODO what do you even do here */
                            }
                        }

                        IMFMediaBuffer_Release(media_buffer);
                    }
                }
            }
        }

        if (new_target_frames == 0) {
            frames_to_do = internal_data->internal_buffer_remaining;
        } else {
            internal_data->internal_buffer_remaining += new_target_frames;
        }
    }
}

OSWRAPPER_AUDIO_DEF size_t oswrapper_audio_get_samples(OSWrapper_audio_spec* audio, short* buffer, size_t frames_to_do) {
    oswrapper_audio__internal_data_win* internal_data = (oswrapper_audio__internal_data_win*) audio->internal_data;
    size_t frame_size = (audio->bits_per_channel / 8) * audio->channel_count;

    /* We have to buffer decoding ourselves */
    if (internal_data->internal_buffer_remaining < (frames_to_do * frame_size)) {
        oswrapper_audio__get_new_samples(audio, frames_to_do * frame_size);
    }

    /* TODO cleanup */
    if (internal_data->internal_buffer_remaining < (frames_to_do * frame_size)) {
        frames_to_do = internal_data->internal_buffer_remaining;
        OSWRAPPER_AUDIO_MEMCPY(buffer, internal_data->internal_buffer + internal_data->internal_buffer_pos, internal_data->internal_buffer_remaining);
        internal_data->internal_buffer_pos += internal_data->internal_buffer_remaining;
        internal_data->internal_buffer_remaining -= internal_data->internal_buffer_remaining;
    } else {
        OSWRAPPER_AUDIO_MEMCPY(buffer, internal_data->internal_buffer + internal_data->internal_buffer_pos, frames_to_do * frame_size);
        internal_data->internal_buffer_pos += (frames_to_do * frame_size);
        internal_data->internal_buffer_remaining -= (frames_to_do * frame_size);
    }

    return frames_to_do;
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
