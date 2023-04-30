/*
OSWrapper audio encoder: Encode audio files with the built in OS audio encoders.

Usage:

TODO

Example:

TODO

Platform requirements:
- On macOS, link with AudioToolbox

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/oswrapper_audio_enc.h
*/

#ifndef OSWRAPPER_INCLUDE_OSWRAPPER_AUDIO_ENC_H
#define OSWRAPPER_INCLUDE_OSWRAPPER_AUDIO_ENC_H
#include <stddef.h>

#ifndef OSWRAPPER_AUDIO_ENC_DEF
#ifdef OSWRAPPER_AUDIO_ENC_STATIC
#define OSWRAPPER_AUDIO_ENC_DEF static
#else
#define OSWRAPPER_AUDIO_ENC_DEF extern
#endif
#endif /* OSWRAPPER_AUDIO_ENC_DEF */

/* You can make these functions return actual booleans if you want */
#ifndef OSWRAPPER_AUDIO_ENC_RESULT_TYPE
#define OSWRAPPER_AUDIO_ENC_RESULT_TYPE int
#endif

#ifndef OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS
#define OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS 1
#endif
#ifndef OSWRAPPER_AUDIO_ENC_RESULT_FAILURE
#define OSWRAPPER_AUDIO_ENC_RESULT_FAILURE 0
#endif

/* Stable-ish API */

/* PCM format enum. */
typedef enum {
    OSWRAPPER_AUDIO_ENC_PCM_INTEGER,
    OSWRAPPER_AUDIO_ENC_PCM_FLOAT
} OSWrapper_audio_enc_pcm_type;

/* PCM endianness type. */
typedef enum {
    OSWRAPPER_AUDIO_ENC_ENDIANNESS_LITTLE,
    OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG
} OSWrapper_audio_enc_pcm_endianness_type;

/* What format the output audio is in.
Note that each format may not be supported on each platform. */
typedef enum {
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSLESS = 0,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY = 1,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC
} OSWrapper_audio_enc_output_type;

/* The created audio context.
TODO */
typedef struct OSWrapper_audio_enc_spec {
    void* internal_data;
    unsigned long sample_rate;
    unsigned int channel_count;
    unsigned int bits_per_channel;
    unsigned int bitrate;
    OSWrapper_audio_enc_pcm_type input_pcm_type;
    OSWrapper_audio_enc_pcm_endianness_type input_pcm_endianness_type;
    OSWrapper_audio_enc_output_type output_type;
} OSWrapper_audio_enc_spec;

/* Call oswrapper_audio_enc_init() before using the library,
and call oswrapper_audio_enc_uninit() after you're done using the library. */
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_init(void);
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_uninit(void);

#ifndef OSWRAPPER_AUDIO_ENC_NO_PATH
/* Finalises and frees resources associated with the given OSWrapper_audio_enc_spec
initialised from oswrapper_audio_enc_make_file_from_path.
Returns 1 on success, or 0 on failure. */
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_finalise_file_context(OSWrapper_audio_enc_spec* audio);

/* Create a sound file encoding context from the given path.
You must set these values on the passed OSWrapper_audio_enc_spec:
- sample_rate: the input PCM sample rate.
- channel_count: the input PCM channel count.
- bits_per_channel: the input PCM bits per channel.
- input_pcm_type: what format the input PCM data is in (integer PCM, floating point PCM).
- input_pcm_endianness_type: the endianness of the input PCM data.
You should also set output_type to the wanted output encoding type.
Note that not all encoding types are supported on every platform,
and some may only be supported at certain sample rates / bits per channel ect.
You can use the preferred lossless or lossy enum cases to use a format
that the current platform should support.
Bitrate is only used for lossy formats, and may be treated as a suggestion.
Returns 1 on success, or 0 on failure. */
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_make_file_from_path(const char* path, OSWrapper_audio_enc_spec* audio);
#endif /* OSWRAPPER_AUDIO_ENC_NO_PATH */

/* Encode audio samples from the given buffer.
Returns 1 on success, or 0 on failure. */
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_encode_samples(OSWrapper_audio_enc_spec* audio, short* buffer, size_t frames_to_do);

#ifdef OSWRAPPER_AUDIO_ENC_IMPLEMENTATION
#ifndef OSWRAPPER_AUDIO_ENC_NO_INCLUDE_STDLIB
#include <stdlib.h>
#endif
#ifndef OSWRAPPER_AUDIO_ENC_NO_INCLUDE_STRING
#include <string.h>
#endif

#ifndef OSWRAPPER_AUDIO_ENC_MALLOC
#define OSWRAPPER_AUDIO_ENC_MALLOC(x) malloc(x)
#endif /* OSWRAPPER_AUDIO_ENC_MALLOC */
#ifndef OSWRAPPER_AUDIO_ENC_FREE
#define OSWRAPPER_AUDIO_ENC_FREE(x) free(x)
#endif /* OSWRAPPER_AUDIO_ENC_FREE */
#ifndef OSWRAPPER_AUDIO_ENC_MEMCPY
#define OSWRAPPER_AUDIO_ENC_MEMCPY(x, y, amount) memcpy(x, y, amount)
#endif /* OSWRAPPER_AUDIO_ENC_MEMCPY */
#ifndef OSWRAPPER_AUDIO_ENC_MEMCMP
#define OSWRAPPER_AUDIO_ENC_MEMCMP(ptr1, ptr2, amount) memcmp(ptr1, ptr2, amount)
#endif /* OSWRAPPER_AUDIO_ENC_MEMCMP */

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4
#if !defined(OSWRAPPER_AUDIO_ENC_USE_AUDIOTOOLBOX_IMPL) && !defined(OSWRAPPER_AUDIO_ENC_NO_USE_AUDIOTOOLBOX_IMPL)
#define OSWRAPPER_AUDIO_ENC_USE_AUDIOTOOLBOX_IMPL
#endif /* !defined(OSWRAPPER_AUDIO_ENC_USE_AUDIOTOOLBOX_IMPL) && !defined(OSWRAPPER_AUDIO_ENC_NO_USE_AUDIOTOOLBOX_IMPL) */
#endif /* defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4 */
#endif /* __APPLE__ */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#if !defined(OSWRAPPER_AUDIO_ENC_USE_WIN_MF_IMPL) && !defined(OSWRAPPER_AUDIO_ENC_NO_USE_WIN_MF_IMPL)
#define OSWRAPPER_AUDIO_ENC_USE_WIN_MF_IMPL
#endif /* !defined(OSWRAPPER_AUDIO_ENC_USE_WIN_MF_IMPL) && !defined(OSWRAPPER_AUDIO_ENC_NO_USE_WIN_MF_IMPL) */
#endif

static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__is_format_lossy(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY:
        return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;

    default:
        return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
    }
}

#ifdef OSWRAPPER_AUDIO_ENC_USE_AUDIOTOOLBOX_IMPL
/* Start macOS AudioToolbox implementation */
#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/AudioFormat.h>
#include <AudioToolbox/ExtendedAudioFile.h>

#include <AvailabilityMacros.h>

#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
#include <CoreServices/CoreServices.h>
#endif

#if !defined(MAC_OS_X_VERSION_10_10) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_10
#define OSWRAPPER_AUDIO_ENC__AUDIO_FORMAT_ID_TYPE UInt32
#else
#define OSWRAPPER_AUDIO_ENC__AUDIO_FORMAT_ID_TYPE AudioFormatID
#endif

#if !defined(MAC_OS_X_VERSION_10_13) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_13
#ifndef kAudioFileFLACType
#define kAudioFileFLACType 'flac'
#endif
#ifndef kAudioFormatFLAC
#define kAudioFormatFLAC 'flac'
#endif
#endif

typedef struct oswrapper_audio_enc__internal_data_mac {
    ExtAudioFileRef audio_file_ext;
} oswrapper_audio_enc__internal_data_mac;

static OSWRAPPER_AUDIO_ENC__AUDIO_FORMAT_ID_TYPE oswrapper_audio_enc__get_audio_format_id_from_enum(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY:
        return kAudioFormatMPEG4AAC;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSLESS:
        return kAudioFormatAppleLossless;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC:
        return kAudioFormatFLAC;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND:
    default:
        return kAudioFormatLinearPCM;
    }
}

static AudioFileTypeID oswrapper_audio_enc__get_audio_file_type_id_from_enum(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV:
        return kAudioFileWAVEType;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC:
        return kAudioFileFLACType;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND:
        return kAudioFileNextType;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC:
        return kAudioFileM4AType;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSLESS:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY:
    default:
        return kAudioFileCAFType;
    }
}

static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__create_desc(AudioStreamBasicDescription* desc, OSWRAPPER_AUDIO_ENC__AUDIO_FORMAT_ID_TYPE format_id, OSWrapper_audio_enc_spec* audio) {
    desc->mFormatID = format_id;
    /* This may be set to 0 when creating compressed formats */
    /* TODO oswrapper_audio_enc__is_format_lossy */
    desc->mSampleRate = format_id == kAudioFormatMPEG4AAC ? 0 : audio->sample_rate;
    desc->mChannelsPerFrame = audio->channel_count;

    if (format_id == kAudioFormatLinearPCM) {
        if (audio->input_pcm_type == OSWRAPPER_AUDIO_ENC_PCM_FLOAT) {
            desc->mFormatFlags = kLinearPCMFormatFlagIsFloat;
        } else if (audio->input_pcm_type == OSWRAPPER_AUDIO_ENC_PCM_INTEGER) {
            desc->mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
        } else {
            /* Unsupported audio format, was not float or integer */
            return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
        }

        desc->mFormatFlags |= kLinearPCMFormatFlagIsPacked;

        if (audio->input_pcm_endianness_type == OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG) {
            desc->mFormatFlags |= kAudioFormatFlagIsBigEndian;
        }

        desc->mBitsPerChannel = audio->bits_per_channel;
        /* kAudioFormatLinearPCM doesn't use packets */
        desc->mFramesPerPacket = 1;
        /* Bytes per channel * channels per frame */
        desc->mBytesPerFrame = (desc->mBitsPerChannel / 8) * desc->mChannelsPerFrame;
        /* Bytes per frame * frames per packet */
        desc->mBytesPerPacket = desc->mBytesPerFrame * desc->mFramesPerPacket;
    } else {
        if (format_id == kAudioFormatAppleLossless || format_id == kAudioFormatFLAC) {
            if (audio->input_pcm_type == OSWRAPPER_AUDIO_ENC_PCM_FLOAT) {
                /* Unsupported input audio format, was not integer */
                return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
            } else {
                switch (audio->bits_per_channel) {
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
                    /* Unsupported input bit depth */
                    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
                }
            }
        } else {
            desc->mFormatFlags = kAudioFormatFlagsAreAllClear;
        }
    }

    UInt32 property_size = sizeof(AudioStreamBasicDescription);

    if (AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &property_size, desc)) {
        /* Could not create valid ASBD from input properties */
        return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_init(void) {
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_uninit(void) {
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

#ifndef OSWRAPPER_AUDIO_ENC_NO_PATH
static OSStatus oswrapper_audio_enc__create_from_path(const char* path, AudioStreamBasicDescription* output_format, ExtAudioFileRef* audio_file, AudioFileTypeID file_type) {
    OSStatus error;
    CFStringRef path_cfstr;
    CFURLRef path_url;
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    FSRef path_fsref;
    const UInt8* base_dir;
    Boolean is_dir;
#endif
    path_cfstr = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
    path_url = CFURLCreateWithFileSystemPath(NULL, path_cfstr, kCFURLPOSIXPathStyle, false);
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    /* TODO Assumes a relative path */
    base_dir = (const UInt8*) "./";
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

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_finalise_file_context(OSWrapper_audio_enc_spec* audio) {
    /* TODO Error checking */
    oswrapper_audio_enc__internal_data_mac* internal_data = (oswrapper_audio_enc__internal_data_mac*) audio->internal_data;
    ExtAudioFileDispose(internal_data->audio_file_ext);
    OSWRAPPER_AUDIO_FREE(audio->internal_data);
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_make_file_from_path(const char* path, OSWrapper_audio_enc_spec* audio) {
#ifdef __cplusplus
    AudioStreamBasicDescription input_format = { };
#else
    AudioStreamBasicDescription input_format = { 0 };
#endif

    if (oswrapper_audio_enc__create_desc(&input_format, kAudioFormatLinearPCM, audio) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
#ifdef __cplusplus
        AudioStreamBasicDescription output_format = { };
#else
        AudioStreamBasicDescription output_format = { 0 };
#endif

        if (oswrapper_audio_enc__create_desc(&output_format, oswrapper_audio_enc__get_audio_format_id_from_enum(audio->output_type), audio) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
            OSStatus error;
            ExtAudioFileRef audio_file_ext = NULL;
            error = oswrapper_audio_enc__create_from_path(path, &output_format, &audio_file_ext, oswrapper_audio_enc__get_audio_file_type_id_from_enum(audio->output_type));

            if (!error) {
                error = ExtAudioFileSetProperty(audio_file_ext, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription), &input_format);

                if (!error) {
                    oswrapper_audio_enc__internal_data_mac* internal_data;
                    UInt32 property_size;
                    AudioConverterRef converter = NULL;
                    property_size = sizeof(AudioConverterRef);

                    if (audio->bitrate != 0 && oswrapper_audio_enc__is_format_lossy(audio->output_type) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS && !ExtAudioFileGetProperty(audio_file_ext, kExtAudioFileProperty_AudioConverter, &property_size, &converter) && converter != NULL) {
                        /* Set encoding bitrate */
                        UInt32 bitrate = audio->bitrate;
                        /* Failure is mostly harmless */
                        AudioConverterSetProperty(converter, kAudioConverterEncodeBitRate, sizeof(bitrate), &bitrate);
                        CFArrayRef converter_config = NULL;
                        ExtAudioFileSetProperty(audio_file_ext, kExtAudioFileProperty_ConverterConfig, sizeof(CFArrayRef), &converter_config);
                    }

                    internal_data = (oswrapper_audio_enc__internal_data_mac*) OSWRAPPER_AUDIO_ENC_MALLOC(sizeof(oswrapper_audio_enc__internal_data_mac));

                    if (internal_data != NULL) {
                        audio->internal_data = (void*) internal_data;
                        internal_data->audio_file_ext = audio_file_ext;
                        return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
                    }
                }

                ExtAudioFileDispose(audio_file_ext);
            }
        }
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_ENC_NO_PATH */

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_encode_samples(OSWrapper_audio_enc_spec* audio, short* buffer, size_t frames_to_do) {
    AudioBufferList buffer_list;
    oswrapper_audio_enc__internal_data_mac* internal_data = (oswrapper_audio_enc__internal_data_mac*) audio->internal_data;
    buffer_list.mNumberBuffers = 1;
    buffer_list.mBuffers[0].mNumberChannels = audio->channel_count;
    buffer_list.mBuffers[0].mDataByteSize = frames_to_do * ((audio->bits_per_channel / 8) * audio->channel_count);
    buffer_list.mBuffers[0].mData = buffer;
    return !ExtAudioFileWrite(internal_data->audio_file_ext, frames_to_do, &buffer_list) ? OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS : OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
/* End macOS AudioToolbox implementation */
#elif defined(OSWRAPPER_AUDIO_ENC_USE_WIN_MF_IMPL)
/* Start Win32 MF implementation */
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_init(void) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_uninit(void) {
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

#ifndef OSWRAPPER_AUDIO_ENC_NO_PATH
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_finalise_file_context(OSWrapper_audio_enc_spec* audio) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_make_file_from_path(const char* path, OSWrapper_audio_enc_spec* audio) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_ENC_NO_PATH */

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_encode_samples(OSWrapper_audio_enc_spec* audio, short* buffer, size_t frames_to_do) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
/* End Win32 MF implementation */
#else
/* No audio loader implementation */
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_init(void) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_uninit(void) {
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

#ifndef OSWRAPPER_AUDIO_ENC_NO_PATH
OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_finalise_file_context(OSWrapper_audio_enc_spec* audio) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_make_file_from_path(const char* path, OSWrapper_audio_enc_spec* audio) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_ENC_NO_PATH */

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_encode_samples(OSWrapper_audio_enc_spec* audio, short* buffer, size_t frames_to_do) {
    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
/* End no audio loader implementation */
#endif
#endif /* OSWRAPPER_AUDIO_ENC_IMPLEMENTATION */
#endif /* OSWRAPPER_INCLUDE_OSWRAPPER_AUDIO_ENC_H */

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
