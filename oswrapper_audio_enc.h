/*
OSWrapper audio encoder: Encode audio files with the built in OS audio encoders.

Usage:

TODO

Example:

TODO

Platform requirements:
- On macOS, link with AudioToolbox
- On Windows, call CoInitialize before using the library,
  and link with mfplat.lib, mfreadwrite.lib, and shlwapi.lib

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
    OSWRAPPER_AUDIO_ENC_PCM_DEFAULT = 0,
    OSWRAPPER_AUDIO_ENC_PCM_INTEGER,
    OSWRAPPER_AUDIO_ENC_PCM_FLOAT
} OSWrapper_audio_enc_pcm_type;

/* PCM endianness type. */
typedef enum {
    OSWRAPPER_AUDIO_ENC_ENDIANNESS_DEFAULT = 0,
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
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC_HE,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC,
    OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC
} OSWrapper_audio_enc_output_type;

typedef struct OSWrapper_audio_enc_format_spec {
    unsigned long sample_rate;
    unsigned int channel_count;
    unsigned int bits_per_channel;
    OSWrapper_audio_enc_pcm_type pcm_type;
    OSWrapper_audio_enc_pcm_endianness_type pcm_endianness_type;
} OSWrapper_audio_enc_format_spec;

/* The created audio context.
TODO */
typedef struct OSWrapper_audio_enc_spec {
    void* internal_data;
    OSWrapper_audio_enc_format_spec input_data;
    OSWrapper_audio_enc_format_spec output_data;
    OSWrapper_audio_enc_output_type output_type;
    unsigned int bitrate;
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
#ifndef OSWRAPPER_AUDIO_ENC_STRLEN
#define OSWRAPPER_AUDIO_ENC_STRLEN(str) strlen(str)
#endif /* OSWRAPPER_AUDIO_ENC_STRLEN */

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
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC_HE:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY:
        return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;

    default:
        return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
    }
}

static OSWrapper_audio_enc_pcm_endianness_type oswrapper_audio_enc__get_endianness_for_format(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV:
        return OSWRAPPER_AUDIO_ENC_ENDIANNESS_LITTLE;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND:
        return OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG;

    default:
        return OSWRAPPER_AUDIO_ENC_ENDIANNESS_DEFAULT;
    }
}

static void oswrapper_audio_enc__fill_output_from_input(OSWrapper_audio_enc_spec* audio) {
    if (audio->input_data.pcm_type == OSWRAPPER_AUDIO_ENC_PCM_DEFAULT) {
        audio->input_data.pcm_type = OSWRAPPER_AUDIO_ENC_PCM_INTEGER;
    }

    if (audio->input_data.pcm_endianness_type == OSWRAPPER_AUDIO_ENC_ENDIANNESS_DEFAULT) {
#if defined(__ppc64__) || defined(__ppc__)
        audio->input_data.pcm_endianness_type = OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG;
#else
        audio->input_data.pcm_endianness_type = OSWRAPPER_AUDIO_ENC_ENDIANNESS_LITTLE;
#endif
    }

#ifdef __APPLE__

    if (!oswrapper_audio_enc__is_format_lossy(audio->output_type))
#endif
    {
        if (audio->output_data.sample_rate == 0) {
            audio->output_data.sample_rate = audio->input_data.sample_rate;
        }
    }

    if (audio->output_data.channel_count == 0) {
        audio->output_data.channel_count = audio->input_data.channel_count;
    }

    if (audio->output_data.bits_per_channel == 0) {
        audio->output_data.bits_per_channel = audio->input_data.bits_per_channel;
    }

    if (audio->output_data.pcm_type == OSWRAPPER_AUDIO_ENC_PCM_DEFAULT) {
        audio->output_data.pcm_type = audio->input_data.pcm_type;
    }

    if (audio->output_data.pcm_endianness_type == OSWRAPPER_AUDIO_ENC_ENDIANNESS_DEFAULT) {
        audio->output_data.pcm_endianness_type = oswrapper_audio_enc__get_endianness_for_format(audio->output_type);

        if (audio->output_data.pcm_endianness_type == OSWRAPPER_AUDIO_ENC_ENDIANNESS_DEFAULT) {
#if defined(__ppc64__) || defined(__ppc__)
            audio->output_data.pcm_endianness_type = OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG;
#else
            audio->output_data.pcm_endianness_type = OSWRAPPER_AUDIO_ENC_ENDIANNESS_LITTLE;
#endif
        }
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

#ifndef kAudioFormatMPEG4AAC_HE
#define kAudioFormatMPEG4AAC_HE 'aach'
#endif
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

static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__is_apple_format_lossy(OSWRAPPER_AUDIO_ENC__AUDIO_FORMAT_ID_TYPE type) {
    /* TODO Add more types */
    switch (type) {
    case kAudioFormatMPEG4AAC:
    case kAudioFormatMPEG4AAC_HE:
        return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;

    default:
        return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
    }
}

static OSWRAPPER_AUDIO_ENC__AUDIO_FORMAT_ID_TYPE oswrapper_audio_enc__get_audio_format_id_from_enum(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY:
        return kAudioFormatMPEG4AAC;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC_HE:
        return kAudioFormatMPEG4AAC_HE;

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
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC_HE:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC:
        return kAudioFileM4AType;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSLESS:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY:
    default:
        return kAudioFileCAFType;
    }
}

static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__create_desc(AudioStreamBasicDescription* desc, OSWRAPPER_AUDIO_ENC__AUDIO_FORMAT_ID_TYPE format_id, OSWrapper_audio_enc_format_spec* audio) {
    desc->mFormatID = format_id;
    /* This may be set to 0 when creating compressed formats */
    desc->mSampleRate = audio->sample_rate;
    desc->mChannelsPerFrame = audio->channel_count;

    if (format_id == kAudioFormatLinearPCM) {
        if (audio->pcm_type == OSWRAPPER_AUDIO_ENC_PCM_FLOAT) {
            desc->mFormatFlags = kLinearPCMFormatFlagIsFloat;
        } else if (audio->pcm_type == OSWRAPPER_AUDIO_ENC_PCM_INTEGER) {
            desc->mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
        } else {
            /* Unsupported audio format, was not float or integer */
            return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
        }

        desc->mFormatFlags |= kLinearPCMFormatFlagIsPacked;

        if (audio->pcm_endianness_type == OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG) {
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
            if (audio->pcm_type == OSWRAPPER_AUDIO_ENC_PCM_FLOAT) {
                /* Source bit depth can't be represented by any flags */
                desc->mFormatFlags = 0;
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
                    /* Source bit depth can't be represented by any flags */
                    desc->mFormatFlags = 0;
                    break;
                }
            }
        } else {
            desc->mFormatFlags = 0;
        }
    }

    UInt32 property_size = sizeof(AudioStreamBasicDescription);

    if (AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &property_size, desc)) {
        /* Could not create valid ASBD from input properties */
        return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__get_bitrates(AudioConverterRef converter, Float64* minimum, Float64* maximum) {
    OSWRAPPER_AUDIO_ENC_RESULT_TYPE return_val;
    UInt32 property_size;
    OSStatus error = AudioConverterGetPropertyInfo(converter, kAudioConverterApplicableEncodeBitRates, &property_size, NULL);
    return_val = OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;

    if (!error && property_size > 0) {
        AudioValueRange* bitrates = (AudioValueRange*) OSWRAPPER_AUDIO_ENC_MALLOC(property_size);

        if (bitrates != NULL) {
            error = AudioConverterGetProperty(converter, kAudioConverterApplicableEncodeBitRates, &property_size, bitrates);

            if (!error) {
                size_t bitrates_amount = property_size / sizeof(AudioValueRange);

                if (bitrates_amount > 0) {
                    size_t i;
                    *minimum = bitrates[0].mMinimum;
                    *maximum = bitrates[0].mMaximum;

                    for (i = 1; i < bitrates_amount; i++) {
                        if (bitrates[i].mMinimum < *minimum) {
                            *minimum = bitrates[i].mMinimum;
                        }

                        if (bitrates[i].mMaximum > *maximum) {
                            *maximum = bitrates[i].mMaximum;
                        }
                    }

                    return_val = OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
                }
            }

            OSWRAPPER_AUDIO_ENC_FREE(bitrates);
        }
    }

    return return_val;
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
    CFURLRef path_url;
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    CFURLRef base_path_url;
#endif
    path_url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*) path, OSWRAPPER_AUDIO_ENC_STRLEN(path), false);
#if !defined(MAC_OS_X_VERSION_10_5) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    error = kAudioFileUnspecifiedError;
    base_path_url = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, path_url);

    if (base_path_url != NULL) {
        CFStringRef out_file_name = CFURLCopyLastPathComponent(path_url);

        if (out_file_name != NULL) {
            FSRef base_path_fsref;

            if (CFURLGetFSRef(base_path_url, &base_path_fsref)) {
                error = ExtAudioFileCreateNew(&base_path_fsref, out_file_name, file_type, output_format, NULL, audio_file);
            }

            CFRelease(out_file_name);
        }

        CFRelease(base_path_url);
    }

#else
    error = ExtAudioFileCreateWithURL(path_url, file_type, output_format, NULL, 0, audio_file);
#endif
    CFRelease(path_url);
    return error;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_finalise_file_context(OSWrapper_audio_enc_spec* audio) {
    /* TODO Error checking */
    oswrapper_audio_enc__internal_data_mac* internal_data = (oswrapper_audio_enc__internal_data_mac*) audio->internal_data;
    ExtAudioFileDispose(internal_data->audio_file_ext);
    OSWRAPPER_AUDIO_ENC_FREE(audio->internal_data);
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_make_file_from_path(const char* path, OSWrapper_audio_enc_spec* audio) {
#ifdef __cplusplus
    AudioStreamBasicDescription input_format = { };
#else
    AudioStreamBasicDescription input_format = { 0 };
#endif
    oswrapper_audio_enc__fill_output_from_input(audio);

    if (oswrapper_audio_enc__create_desc(&input_format, kAudioFormatLinearPCM, &audio->input_data) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
#ifdef __cplusplus
        AudioStreamBasicDescription output_format = { };
#else
        AudioStreamBasicDescription output_format = { 0 };
#endif

        if (oswrapper_audio_enc__create_desc(&output_format, oswrapper_audio_enc__get_audio_format_id_from_enum(audio->output_type), &audio->output_data) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
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
                        Float64 minimum;
                        Float64 maximum;
                        /* Set encoding bitrate */
                        UInt32 bitrate = audio->bitrate;

                        if (oswrapper_audio_enc__get_bitrates(converter, &minimum, &maximum) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
                            /* Clamp bitrate to allowed values */
                            if (bitrate > (UInt32) maximum) {
                                bitrate = (UInt32) maximum;
                            }

                            if (bitrate < (UInt32) minimum) {
                                bitrate = (UInt32) minimum;
                            }
                        }

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
    buffer_list.mBuffers[0].mNumberChannels = audio->input_data.channel_count;
    buffer_list.mBuffers[0].mDataByteSize = frames_to_do * ((audio->input_data.bits_per_channel / 8) * audio->input_data.channel_count);
    buffer_list.mBuffers[0].mData = buffer;
    return !ExtAudioFileWrite(internal_data->audio_file_ext, frames_to_do, &buffer_list) ? OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS : OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
/* End macOS AudioToolbox implementation */
#elif defined(OSWRAPPER_AUDIO_ENC_USE_WIN_MF_IMPL)
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

#if WINVER < 0x0A00
DEFINE_MEDIATYPE_GUID(MFAudioFormat_FLAC, 0xF1AC);
DEFINE_MEDIATYPE_GUID(MFAudioFormat_ALAC, 0x6C61);
#endif

/* Using CINTERFACE breaks some headers, so we have to define these ourselves */
#if defined(__cplusplus) && !defined(CINTERFACE) && !defined(OSWRAPPER_AUDIO_NO_DEFINE_WINMF_C_INTERFACE)
#ifndef IMFMediaBuffer_Lock
#define IMFMediaBuffer_Lock(media_buffer, ...) media_buffer->Lock(__VA_ARGS__)
#endif
#ifndef IMFMediaBuffer_Release
#define IMFMediaBuffer_Release(media_buffer) media_buffer->Release()
#endif
#ifndef IMFMediaBuffer_Unlock
#define IMFMediaBuffer_Unlock(media_buffer) media_buffer->Unlock()
#endif
#ifndef IMFMediaType_Release
#define IMFMediaType_Release(media_type) media_type->Release()
#endif
#ifndef IMFMediaType_SetGUID
#define IMFMediaType_SetGUID(media_type, ...) media_type->SetGUID(__VA_ARGS__)
#endif
#ifndef IMFMediaType_SetUINT32
#define IMFMediaType_SetUINT32(media_type, ...) media_type->SetUINT32(__VA_ARGS__)
#endif
#ifndef IMFSample_Release
#define IMFSample_Release(sample) sample->Release()
#endif
#ifndef IMFSample_AddBuffer
#define IMFSample_AddBuffer(sample, ...) sample->AddBuffer(__VA_ARGS__)
#endif
#ifndef IMFMediaBuffer_SetCurrentLength
#define IMFMediaBuffer_SetCurrentLength(media_buffer, ...) media_buffer->SetCurrentLength(__VA_ARGS__)
#endif
#ifndef IMFSample_SetSampleDuration
#define IMFSample_SetSampleDuration(sample, ...) sample->SetSampleDuration(__VA_ARGS__)
#endif
#ifndef IMFSample_SetSampleTime
#define IMFSample_SetSampleTime(sample, ...) sample->SetSampleTime(__VA_ARGS__)
#endif
#ifndef IMFSinkWriter_WriteSample
#define IMFSinkWriter_WriteSample(sink_writer, ...) sink_writer->WriteSample(__VA_ARGS__)
#endif
#ifndef IMFSinkWriter_AddStream
#define IMFSinkWriter_AddStream(sink_writer, ...) sink_writer->AddStream(__VA_ARGS__)
#endif
#ifndef IMFSinkWriter_SetInputMediaType
#define IMFSinkWriter_SetInputMediaType(sink_writer, ...) sink_writer->SetInputMediaType(__VA_ARGS__)
#endif
#ifndef IMFSinkWriter_BeginWriting
#define IMFSinkWriter_BeginWriting(sink_writer) sink_writer->BeginWriting()
#endif
#ifndef IMFSinkWriter_Finalize
#define IMFSinkWriter_Finalize(sink_writer) sink_writer->Finalize()
#endif
#ifndef IMFSinkWriter_Release
#define IMFSinkWriter_Release(sink_writer) sink_writer->Release()
#endif
#endif

/* TODO Ugly hack */
#ifndef OSWRAPPER_AUDIO_ENC_PATH_MAX
#define OSWRAPPER_AUDIO_ENC_PATH_MAX MAX_PATH
#endif

#ifdef OSWRAPPER_AUDIO_ENC_MANAGE_COINIT
#include <objbase.h>

#ifndef OSWRAPPER_AUDIO_ENC_COINIT_VALUE
#define OSWRAPPER_AUDIO_ENC_COINIT_VALUE COINIT_MULTITHREADED
#endif
#endif

/* The startup flags for MFStartup */
#ifndef OSWRAPPER_AUDIO_ENC__MF_STARTUP_VAL
#define OSWRAPPER_AUDIO_ENC__MF_STARTUP_VAL MFSTARTUP_LITE
#endif

typedef struct oswrapper_audio_enc__internal_data_win {
    IMFSinkWriter* writer;
    DWORD audio_stream_index;
    LONGLONG output_pos;
} oswrapper_audio_enc__internal_data_win;

static GUID oswrapper_audio_enc__get_guid_from_enum(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC_HE:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSY:
        return MFAudioFormat_AAC;

    /*case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_MP3:
        return MFAudioFormat_MP3;*/

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC:
        return MFAudioFormat_ALAC;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC:
        return MFAudioFormat_FLAC;

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_PREFERRED_LOSSLESS:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV:
    default:
        return MFAudioFormat_PCM;
    }
}

#define OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(X) if (FAILED(X)) { goto cleanup; }

static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__make_media_type_for_input_format(IMFMediaType** input_media_type, OSWrapper_audio_enc_format_spec* audio_spec) {
    if (SUCCEEDED(MFCreateMediaType(input_media_type))) {
        IMFMediaType* media_type = *input_media_type;
#ifdef __cplusplus
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_MAJOR_TYPE, MFMediaType_Audio));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_SUBTYPE, audio_spec->pcm_type == OSWRAPPER_AUDIO_ENC_PCM_FLOAT ? MFAudioFormat_Float : MFAudioFormat_PCM));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
#else
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_SUBTYPE, audio_spec->pcm_type == OSWRAPPER_AUDIO_ENC_PCM_FLOAT ? &MFAudioFormat_Float : &MFAudioFormat_PCM));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
#endif
        return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
cleanup:
        IMFMediaType_Release(media_type);
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}

static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__make_media_type_for_output_format(IMFMediaType** output_media_type, OSWrapper_audio_enc_format_spec* audio_spec, OSWrapper_audio_enc_output_type output_type, unsigned int bitrate) {
    if (SUCCEEDED(MFCreateMediaType(output_media_type))) {
        GUID output_format_guid;
        IMFMediaType* media_type = *output_media_type;
        output_format_guid = oswrapper_audio_enc__get_guid_from_enum(output_type);
#ifdef __cplusplus
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_MAJOR_TYPE, MFMediaType_Audio));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_SUBTYPE, output_format_guid));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));

        if (bitrate != 0 && oswrapper_audio_enc__is_format_lossy(output_type) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
            OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bitrate / 8));
        }

#else
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_SUBTYPE, &output_format_guid));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));

        if (bitrate != 0 && oswrapper_audio_enc__is_format_lossy(output_type) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
            OSWRAPPER_AUDIO_ENC__MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bitrate / 8));
        }

#endif
        return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
cleanup:
        IMFMediaType_Release(media_type);
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_init(void) {
#ifdef OSWRAPPER_AUDIO_ENC_MANAGE_COINIT
    HRESULT result = CoInitializeEx(NULL, OSWRAPPER_AUDIO_ENC_COINIT_VALUE);

    if (SUCCEEDED(result)) {
        result = MFStartup(MF_VERSION, OSWRAPPER_AUDIO_ENC__MF_STARTUP_VAL);

        if (SUCCEEDED(result)) {
            return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
        }

        CoUninitialize();
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
#else
    return SUCCEEDED(MFStartup(MF_VERSION, OSWRAPPER_AUDIO_ENC__MF_STARTUP_VAL)) ? OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS : OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
#endif
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_uninit(void) {
#ifdef OSWRAPPER_AUDIO_ENC_MANAGE_COINIT

    if (FAILED(MFShutdown())) {
        return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
    }

    CoUninitialize();
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
#else
    return SUCCEEDED(MFShutdown()) ? OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS : OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
#endif
}

#ifndef OSWRAPPER_AUDIO_ENC_NO_PATH
static OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc__make_sink_writer_from_path(const char* path, IMFSinkWriter** writer) {
    HRESULT result;
    /* TODO Ugly hack */
    wchar_t path_buffer[OSWRAPPER_AUDIO_ENC_PATH_MAX];
    result = MultiByteToWideChar(CP_UTF8, 0, path, -1, path_buffer, OSWRAPPER_AUDIO_ENC_PATH_MAX);

    if (SUCCEEDED(result)) {
        result = MFCreateSinkWriterFromURL(path_buffer, NULL, NULL, writer);

        if (SUCCEEDED(result)) {
            return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
        }
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_finalise_file_context(OSWrapper_audio_enc_spec* audio) {
    /* TODO Error checking */
    oswrapper_audio_enc__internal_data_win* internal_data = (oswrapper_audio_enc__internal_data_win*) audio->internal_data;
    IMFSinkWriter_Finalize(internal_data->writer);
    IMFSinkWriter_Release(internal_data->writer);
    OSWRAPPER_AUDIO_ENC_FREE(audio->internal_data);
    return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
}

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_make_file_from_path(const char* path, OSWrapper_audio_enc_spec* audio) {
    IMFSinkWriter* writer;
    oswrapper_audio_enc__fill_output_from_input(audio);

    if (oswrapper_audio_enc__make_sink_writer_from_path(path, &writer)) {
        /* Output stream format */
        IMFMediaType* output_media_type;

        if (oswrapper_audio_enc__make_media_type_for_output_format(&output_media_type, &audio->output_data, audio->output_type, audio->bitrate) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
            DWORD audio_stream_index;
            HRESULT result = IMFSinkWriter_AddStream(writer, output_media_type, &audio_stream_index);
            IMFMediaType_Release(output_media_type);

            if (SUCCEEDED(result)) {
                /* Input stream format */
                IMFMediaType* input_media_type;

                if (oswrapper_audio_enc__make_media_type_for_input_format(&input_media_type, &audio->input_data) == OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS) {
                    result = IMFSinkWriter_SetInputMediaType(writer, audio_stream_index, input_media_type, NULL);
                    IMFMediaType_Release(input_media_type);

                    if (SUCCEEDED(result)) {
                        /* Initialise sink writer */
                        result = IMFSinkWriter_BeginWriting(writer);

                        if (SUCCEEDED(result)) {
                            oswrapper_audio_enc__internal_data_win* internal_data = (oswrapper_audio_enc__internal_data_win*) OSWRAPPER_AUDIO_ENC_MALLOC(sizeof(oswrapper_audio_enc__internal_data_win));

                            if (internal_data != NULL) {
                                audio->internal_data = (void*) internal_data;
                                internal_data->writer = writer;
                                internal_data->audio_stream_index = audio_stream_index;
                                internal_data->output_pos = 0;
                                return OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
                            }

                            IMFSinkWriter_Finalize(writer);
                        }
                    }
                }
            }
        }

        IMFSinkWriter_Release(writer);
    }

    return OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
}
#endif /* OSWRAPPER_AUDIO_ENC_NO_PATH */

#define OSWRAPPER_AUDIO_ENC__WRITE_TO_BUFFER_HELPER_FAIL(X) if (FAILED(X)) { goto fast_fail; }

OSWRAPPER_AUDIO_ENC_DEF OSWRAPPER_AUDIO_ENC_RESULT_TYPE oswrapper_audio_enc_encode_samples(OSWrapper_audio_enc_spec* audio, short* buffer, size_t frames_to_do) {
    OSWRAPPER_AUDIO_ENC_RESULT_TYPE return_val;
    HRESULT result;
    DWORD current_length;
    BYTE* sample_audio_data;
    LONGLONG this_dur;
    IMFSample* sample;
    IMFMediaBuffer* media_buffer;
    size_t frame_size;
    size_t this_multi;
    oswrapper_audio_enc__internal_data_win* internal_data = (oswrapper_audio_enc__internal_data_win*) audio->internal_data;
    /* Create IMFSample */
    sample = NULL;
    media_buffer = NULL;
    sample_audio_data = NULL;
    this_dur = 0;
    return_val = OSWRAPPER_AUDIO_ENC_RESULT_FAILURE;
    frame_size = ((audio->input_data.bits_per_channel / 8) * audio->input_data.channel_count);
    this_multi = frames_to_do * frame_size;
    OSWRAPPER_AUDIO_ENC__WRITE_TO_BUFFER_HELPER_FAIL(MFCreateSample(&sample));
    OSWRAPPER_AUDIO_ENC__WRITE_TO_BUFFER_HELPER_FAIL(MFCreateMemoryBuffer(this_multi, &media_buffer));
    OSWRAPPER_AUDIO_ENC__WRITE_TO_BUFFER_HELPER_FAIL(IMFSample_AddBuffer(sample, media_buffer));
    result = IMFMediaBuffer_Lock(media_buffer, &sample_audio_data, &current_length, NULL);

    if (SUCCEEDED(result)) {
        OSWRAPPER_AUDIO_ENC_MEMCPY(sample_audio_data, buffer, this_multi);
        IMFMediaBuffer_Unlock(media_buffer);
        result = IMFMediaBuffer_SetCurrentLength(media_buffer, this_multi);

        if (SUCCEEDED(result)) {
            this_dur = MFllMulDiv(frames_to_do, 10000000ULL, audio->input_data.sample_rate, 0);
            result = IMFSample_SetSampleDuration(sample, this_dur);

            if (SUCCEEDED(result)) {
                result = IMFSample_SetSampleTime(sample, internal_data->output_pos);

                if (SUCCEEDED(result)) {
                    result = IMFSinkWriter_WriteSample(internal_data->writer, internal_data->audio_stream_index, sample);

                    if (SUCCEEDED(result)) {
                        internal_data->output_pos += this_dur;
                        return_val = OSWRAPPER_AUDIO_ENC_RESULT_SUCCESS;
                    }
                }
            }
        }
    }

fast_fail:

    if (sample != NULL) {
        IMFSample_Release(sample);
    }

    if (media_buffer != NULL) {
        IMFMediaBuffer_Release(media_buffer);
    }

    return return_val;
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
