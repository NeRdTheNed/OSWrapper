/*
This program uses oswrapper_audio to decode and play an audio file twice,
using macOS APIs for sound output.

Usage: demo_oswrapper_audio_mac (audio_file.ext)
If no input is provided, it will play the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/demo_oswrapper_audio_mac.c
*/

#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"
#include <stdlib.h>

#include <pthread.h>

#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>

#include <AvailabilityMacros.h>

#if !defined(MAC_OS_X_VERSION_10_6) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
/* These deprecated Carbon APIs were replaced with equivalent functions and types in macOS 10.6 */
#include <CoreServices/CoreServices.h>
#define AudioComponent Component
#define AudioComponentDescription ComponentDescription
#define AudioComponentFindNext FindNextComponent
#define AudioComponentInstanceDispose CloseComponent
#define AudioComponentInstanceNew OpenAComponent
#endif

#ifdef HINT_OUTPUT_FORMAT
#define SAMPLE_RATE 44100
#define CHANNEL_COUNT 2
#define BITS_PER_CHANNEL 16
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER
#define ENDIANNESS_TYPE OSWRAPPER_AUDIO_ENDIANNESS_NOT_SPECIFIED
#else
#define SAMPLE_RATE 0
#define CHANNEL_COUNT 0
#define BITS_PER_CHANNEL 0
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_NOT_SET
#define ENDIANNESS_TYPE OSWRAPPER_AUDIO_ENDIANNESS_NOT_SPECIFIED
#endif

#define FAIL_WITH_MESSAGE_ON_COND(cond, message) if ((cond)) { puts(message); return EXIT_FAILURE; }

/* TODO Janky threads */
static pthread_mutex_t pthread_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pthread_condition = PTHREAD_COND_INITIALIZER;
static int audio_done = 0;

static OSStatus Callback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) {
    int amount_done;
    OSWrapper_audio_spec* audio_spec = (OSWrapper_audio_spec*)inRefCon;
    (void)ioActionFlags;
    (void)inTimeStamp;
    (void)inBusNumber;

    if (audio_spec == NULL) {
        return 0;
    }

    /* Because the stream is interleaved, there is only one buffer */
    amount_done = oswrapper_audio_get_samples(audio_spec, (short*)ioData->mBuffers[0].mData, inNumberFrames);

    if (amount_done == 0) {
        audio_done = 1;
        pthread_cond_signal(&pthread_condition);
    }

    return 0;
}

int main(int argc, char** argv) {
    FAIL_WITH_MESSAGE_ON_COND(!oswrapper_audio_init(), "Could not initialise oswrapper_audio!");
    /* Allocate memory for an OSWrapper_audio_spec */
    OSWrapper_audio_spec* audio_spec = (OSWrapper_audio_spec*) calloc(1, sizeof(OSWrapper_audio_spec));
    FAIL_WITH_MESSAGE_ON_COND((audio_spec == NULL), "calloc failed!");
    /* Hint the desired output format */
    audio_spec->sample_rate = SAMPLE_RATE;
    audio_spec->channel_count = CHANNEL_COUNT;
    audio_spec->bits_per_channel = BITS_PER_CHANNEL;
    audio_spec->audio_type = AUDIO_FORMAT;
    audio_spec->endianness_type = ENDIANNESS_TYPE;
    /* Or set these values to zero to use the input format's values.
    The values in audio_spec will always be set to the output format's values
    after initialising an OSWrapper_audio_spec. */
    /* Load audio with desired format */
    int did_load_audio = oswrapper_audio_load_from_path(argc < 2 ? "noise.wav" : argv[argc - 1], audio_spec);
    FAIL_WITH_MESSAGE_ON_COND((did_load_audio == 0), "oswrapper_audio_load_from_path failed!");
    /* audio_spec now contains the output format values. */
    /* macOS: Find the default output AudioUnit */
    AudioComponentDescription default_output_description;
    default_output_description.componentType = kAudioUnitType_Output;
    default_output_description.componentSubType = kAudioUnitSubType_DefaultOutput;
    default_output_description.componentManufacturer = kAudioUnitManufacturer_Apple;
    default_output_description.componentFlags = 0;
    default_output_description.componentFlagsMask = 0;
    AudioComponent default_output_component = AudioComponentFindNext(NULL, &default_output_description);
    FAIL_WITH_MESSAGE_ON_COND((default_output_component == NULL), "Could not find default output component!");
    /* macOS: Create an instance of the default output audio unit */
    AudioUnit audio_unit;
    OSStatus error = AudioComponentInstanceNew(default_output_component, &audio_unit);
    FAIL_WITH_MESSAGE_ON_COND((error), "Could not make a new instance of the default output component!");
    /* macOS: Use a suitable output format */
    AudioStreamBasicDescription want;
    want.mFormatID = kAudioFormatLinearPCM;

    if (audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
        want.mFormatFlags = kLinearPCMFormatFlagIsFloat;
    } else if (audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_INTEGER) {
        want.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger;
    } else {
        puts("Unsupported audio format, was not float or integer!");
        return EXIT_FAILURE;
    }

    want.mFormatFlags |= kLinearPCMFormatFlagIsPacked;

    if (audio_spec->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG) {
        want.mFormatFlags |= kAudioFormatFlagIsBigEndian;
    }

    want.mSampleRate = audio_spec->sample_rate;
    want.mBitsPerChannel = audio_spec->bits_per_channel;
    want.mChannelsPerFrame = audio_spec->channel_count;
    /* kAudioFormatLinearPCM doesn't use packets */
    want.mFramesPerPacket = 1;
    /* Bytes per channel * channels per frame */
    want.mBytesPerFrame = (want.mBitsPerChannel / 8) * want.mChannelsPerFrame;
    /* Bytes per frame * frames per packet */
    want.mBytesPerPacket = want.mBytesPerFrame * want.mFramesPerPacket;
    error = AudioUnitSetProperty(audio_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &want, sizeof(AudioStreamBasicDescription));
    FAIL_WITH_MESSAGE_ON_COND((error), "Could not set kAudioUnitProperty_StreamFormat!");
    /* macOS: Set the AudioUnit output callback */
    AURenderCallbackStruct callback;
    /* Callback function */
    callback.inputProc = Callback;
    /* User data */
    callback.inputProcRefCon = audio_spec;
    error = AudioUnitSetProperty(audio_unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(AURenderCallbackStruct));
    FAIL_WITH_MESSAGE_ON_COND((error), "Could not set kAudioUnitProperty_SetRenderCallback!");
    /* macOS: Initialise the configured AudioUnit instance */
    error = AudioUnitInitialize(audio_unit);
    FAIL_WITH_MESSAGE_ON_COND((error), "Could not initialise audio unit!");
    /* macOS: Start the callback function to play audio */
    error = AudioOutputUnitStart(audio_unit);
    FAIL_WITH_MESSAGE_ON_COND((error), "Could not start audio unit!");
    /* Wait until audio playback is done TODO Janky threads */
    puts("Playing sound...");
    pthread_mutex_lock(&pthread_mutex);

    while (!audio_done) {
        pthread_cond_wait(&pthread_condition, &pthread_mutex);
    }

    pthread_mutex_unlock(&pthread_mutex);
#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
    /* Print current location for test purposes */
    OSWRAPPER_AUDIO_SEEK_TYPE pos = 0;
    int didGetPos = oswrapper_audio_get_pos(audio_spec, &pos);
    FAIL_WITH_MESSAGE_ON_COND((!didGetPos), "Failed to get current position!");
    printf("End of sound position was %lli\n", pos);
#endif
    /* Replay sound */
    AudioOutputUnitStop(audio_unit);
#ifdef OSWRAPPER_AUDIO_EXPERIMENTAL
    /* For testing purposes. Equivalent to oswrapper_audio_rewind */
    oswrapper_audio_seek(audio_spec, 0);
#else
    oswrapper_audio_rewind(audio_spec);
#endif
    audio_done = 0;
    AudioOutputUnitStart(audio_unit);
    puts("Playing sound again...");
    pthread_mutex_lock(&pthread_mutex);

    while (!audio_done) {
        pthread_cond_wait(&pthread_condition, &pthread_mutex);
    }

    pthread_mutex_unlock(&pthread_mutex);
    puts("Finished playing sound!");
    /* Cleanup */
    /* macOS: Free audio unit */
    AudioOutputUnitStop(audio_unit);
    AudioUnitSetProperty(audio_unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, NULL, 0);
    AudioUnitUninitialize(audio_unit);
    AudioComponentInstanceDispose(audio_unit);
    /* Destroy mutex and condition */
    pthread_mutex_destroy(&pthread_mutex);
    pthread_cond_destroy(&pthread_condition);
    /* Free audio file */
    int did_close = oswrapper_audio_free_context(audio_spec);
    FAIL_WITH_MESSAGE_ON_COND((!did_close), "Failed to free sound context!");
    FAIL_WITH_MESSAGE_ON_COND(!oswrapper_audio_uninit(), "Could not uninitialise oswrapper_audio!");
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
