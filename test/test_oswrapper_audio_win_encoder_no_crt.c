/*
This program uses oswrapper_audio to decode an audio file,
and encodes the decoded PCM data to WAV using Windows APIs.

This is a modified version of test_oswrapper_audio_win_encoder to allow compiling on Windows
without using a C runtime (abbreviated to CRT).

Usage: test_oswrapper_audio_win_encoder_no_crt (audio_file.ext)
If no input is provided, it will decode and encode the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_audio_win_encoder_no_crt.c
*/

#define OSWRAPPER_AUDIO_MALLOC(x) HeapAlloc(GetProcessHeap(), 0, x)
#define OSWRAPPER_AUDIO_FREE(x) HeapFree(GetProcessHeap(), 0, x)
/* These are just macros for the C functions, so we have to implement our own versions :/
#define OSWRAPPER_AUDIO_MEMCPY(x, y, amount) CopyMemory(x, y, amount)*/
#include <stddef.h>

/* Forward memcpy. Not optimised. */
static void* bad_memcpy(void* destination, const void* source, size_t num) {
    unsigned char* dest_cast = (unsigned char*) destination;
    unsigned char* source_cast = (unsigned char*) source;
    size_t i;

    for (i = 0; i < num; i++) {
        dest_cast[i] = source_cast[i];
    }

    return destination;
}
#define OSWRAPPER_AUDIO_MEMCPY(x, y, amount) bad_memcpy(x, y, amount)
static int impl_memcmp(const void* ptr1, const void* ptr2, size_t amount) {
    const unsigned char* cast_1 = (const unsigned char*) ptr1;
    const unsigned char* cast_2 = (const unsigned char*) ptr2;

    while (amount-- > 0) {
        if (*cast_1++ != *cast_2++) {
            return cast_1[-1] < cast_2[-1] ? -1 : 1;
        }
    }

    return 0;
}
#define OSWRAPPER_AUDIO_MEMCMP(ptr1, ptr2, amount) impl_memcmp(ptr1, ptr2, amount)
#define OSWRAPPER_AUDIO_STATIC
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#include <objbase.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

#include <shellapi.h>
/* Link with libraries to replace CRT functions. */
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
/* Linking bodge */
int _fltused = 0;
#define AUDIO_DEMO_STRLEN(x) lstrlen(x)
#define AUDIO_DEMO_CONSOLE_OUTPUT(x) WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), x, AUDIO_DEMO_STRLEN(x), NULL, NULL)
/* HACK: The maximum characters this function writes to a buffer is 1025,
so as long as the buffer is at least 1025 characters, it's "safe" to use. */
#define AUDIO_DEMO_SNPRINTF(buffer, len, format, ...) wsprintfA(buffer, format, __VA_ARGS__)
#define AUDIO_DEMO_FILE_TYPE HANDLE
#define AUDIO_DEMO_FILE_NUM_WRITTEN_TYPE DWORD
#define AUDIO_DEMO_INVALID_FILE_TYPE INVALID_HANDLE_VALUE
#define AUDIO_DEMO_FILE_OPEN_READ(path) CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
#define AUDIO_DEMO_FILE_OPEN_WRITE(path) CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL)
#define AUDIO_DEMO_FILE_WRITE(num_written, mem, elem_size, elem, file) WriteFile(file, mem, (elem_size * elem), &num_written, NULL)
#define AUDIO_DEMO_FILE_CLOSE(x) CloseHandle(x)
#define AUDIO_DEMO_CALLOC(x, size) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (x * size))
#define AUDIO_DEMO_FORMAT_SIZE_T "%d"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

//#include <stdio.h>
//#include <stdlib.h>

#if defined(__cplusplus) && !defined(CINTERFACE)
#define IMFSample_AddBuffer(sample, ...) sample->AddBuffer(__VA_ARGS__)
#define IMFMediaBuffer_SetCurrentLength(media_buffer, ...) media_buffer->SetCurrentLength(__VA_ARGS__)
#define IMFSample_SetSampleDuration(sample, ...) sample->SetSampleDuration(__VA_ARGS__)
#define IMFSample_SetSampleTime(sample, ...) sample->SetSampleTime(__VA_ARGS__)
#define IMFSinkWriter_WriteSample(sink_writer, ...) sink_writer->WriteSample(__VA_ARGS__)
#define IMFSinkWriter_AddStream(sink_writer, ...) sink_writer->AddStream(__VA_ARGS__)
#define IMFSinkWriter_SetInputMediaType(sink_writer, ...) sink_writer->SetInputMediaType(__VA_ARGS__)
#define IMFSinkWriter_BeginWriting(sink_writer) sink_writer->BeginWriting()
#define IMFSinkWriter_Finalize(sink_writer) sink_writer->Finalize()
#define IMFSinkWriter_Release(sink_writer) sink_writer->Release()
#endif

#define AUDIO_DEMO_PRINT_BUFFER_SIZE 1025
static char print_buffer[AUDIO_DEMO_PRINT_BUFFER_SIZE] = "";

/* TODO ugly hack */
#define BODGE_ERROR_STR "Error"
static char error_string[sizeof(BODGE_ERROR_STR)] = BODGE_ERROR_STR;
static char* argv_error[2];

int main(int argc, char** argv);

int mainCRTStartup(void) {
    /* TODO better argument parsing */
    int argc = 0;
    char** argv = NULL;
    /* Get arguments as LPWSTR* */
    LPWSTR* win_argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (win_argv != NULL && argc != 0) {
        /* Allocate argv */
        argv = (char**) OSWRAPPER_AUDIO_MALLOC((argc + 1) * sizeof(char*));

        if (argv != NULL) {
            for (int i = 0; i < argc; i++) {
                /* Get needed size for argument string */
                int wide_length = WideCharToMultiByte(CP_UTF8, 0, win_argv[i], -1, NULL, 0, NULL, NULL);

                if (wide_length > 0) {
                    argv[i] = OSWRAPPER_AUDIO_MALLOC(wide_length * sizeof(char));

                    if (argv[i] != NULL) {
                        /* Convert the LPWSTR argument */
                        WideCharToMultiByte(CP_UTF8, 0, win_argv[i], -1, argv[i], wide_length, NULL, NULL);
                    } else {
                        /* Malloc failed, pretend this string is the real argument */
                        argv[i] = error_string;
                    }
                } else {
                    /* WideCharToMultiByte failed, pretend this string is the real argument */
                    argv[i] = error_string;
                }
            }
        } else {
            /* Malloc failed, just pretend we didn't have any arguments */
            argc = 0;
            argv = argv_error;
            argv_error[0] = error_string;
            argv_error[1] = NULL;
        }
    } else {
        /* CommandLineToArgvW failed, just pretend we didn't have any arguments */
        argc = 0;
        argv = argv_error;
        argv_error[0] = error_string;
        argv_error[1] = NULL;
    }

    /* Free the result of CommandLineToArgvW */
    if (win_argv != NULL) {
        LocalFree(win_argv);
    }

    int return_val = main(argc, argv);

    /* Free argument strings */
    if (argv != NULL && argc != 0) {
        for (int i = 0; i < argc; i++) {
            if (argv[i] != error_string) {
                OSWRAPPER_AUDIO_FREE(argv[i]);
            }
        }
    }

    /* Free argv */
    if (argv != NULL && argv != argv_error) {
        OSWRAPPER_AUDIO_FREE(argv);
    }

    ExitProcess(return_val);
}

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

#if !defined(DEMO_WIN_CONVERT_TO_M4A) && !defined(DEMO_WIN_CONVERT_TO_WAV) && !defined(DEMO_WIN_CONVERT_TO_MP3) && !defined(DEMO_WIN_CONVERT_TO_FLAC)
#define DEMO_WIN_CONVERT_TO_WAV
#endif

#ifdef DEMO_WIN_CONVERT_TO_M4A
#define DEMO_WIN_EXT ".m4a"
#ifdef __cplusplus
#define DEMO_WIN_CONV_FORMAT MFAudioFormat_AAC
#else
#define DEMO_WIN_CONV_FORMAT &MFAudioFormat_AAC
#endif
#elif defined(DEMO_WIN_CONVERT_TO_WAV)
#define DEMO_WIN_EXT ".wav"
#ifdef __cplusplus
#define DEMO_WIN_CONV_FORMAT MFAudioFormat_PCM
#else
#define DEMO_WIN_CONV_FORMAT &MFAudioFormat_PCM
#endif
#elif defined(DEMO_WIN_CONVERT_TO_MP3)
#define DEMO_WIN_EXT ".mp3"
#ifdef __cplusplus
#define DEMO_WIN_CONV_FORMAT MFAudioFormat_MP3
#else
#define DEMO_WIN_CONV_FORMAT &MFAudioFormat_MP3
#endif
#elif defined(DEMO_WIN_CONVERT_TO_FLAC)
#define DEMO_WIN_EXT ".flac"
#ifdef __cplusplus
#define DEMO_WIN_CONV_FORMAT MFAudioFormat_FLAC
#else
#define DEMO_WIN_CONV_FORMAT &MFAudioFormat_FLAC
#endif
#else
#error No format defined
#endif

#define DEMO_ENC_XSTR(X) DEMO_ENC_STR(X)
#define DEMO_ENC_STR(X) #X

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

#ifdef _DEBUG
#define DEMO_MAKE_MEDIA_HELPER(X) if (FAILED(X)) { AUDIO_DEMO_CONSOLE_OUTPUT(DEMO_ENC_XSTR(X) " failed!"); goto cleanup; }
#else
#define DEMO_MAKE_MEDIA_HELPER(X) if (FAILED(X)) { goto cleanup; }
#endif

static OSWRAPPER_AUDIO_RESULT_TYPE make_media_type_for_input_format(IMFMediaType** input_media_type, OSWrapper_audio_spec* audio_spec) {
    if (SUCCEEDED(MFCreateMediaType(input_media_type))) {
        IMFMediaType* media_type = *input_media_type;
#ifdef __cplusplus
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_MAJOR_TYPE, MFMediaType_Audio));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_SUBTYPE, audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT ? MFAudioFormat_Float : MFAudioFormat_PCM));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
#else
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_SUBTYPE, audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT ? &MFAudioFormat_Float : &MFAudioFormat_PCM));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
#endif
        return OSWRAPPER_AUDIO_RESULT_SUCCESS;
cleanup:
        IMFMediaType_Release(media_type);
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

static OSWRAPPER_AUDIO_RESULT_TYPE make_media_type_for_output_format(IMFMediaType** output_media_type, OSWrapper_audio_spec* audio_spec) {
    if (SUCCEEDED(MFCreateMediaType(output_media_type))) {
        IMFMediaType* media_type = *output_media_type;
#ifdef __cplusplus
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_MAJOR_TYPE, MFMediaType_Audio));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, MF_MT_SUBTYPE, DEMO_WIN_CONV_FORMAT));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
#ifdef DEMO_WIN_CONVERT_TO_M4A
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 128 * 1000 / 8));
#elif defined(DEMO_WIN_CONVERT_TO_MP3)
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 320 * 1000 / 8));
#endif
#else
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetGUID(media_type, &MF_MT_SUBTYPE, DEMO_WIN_CONV_FORMAT));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_BITS_PER_SAMPLE, audio_spec->bits_per_channel));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_SAMPLES_PER_SECOND, audio_spec->sample_rate));
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_NUM_CHANNELS, audio_spec->channel_count));
#ifdef DEMO_WIN_CONVERT_TO_M4A
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 128 * 1000 / 8));
#elif defined(DEMO_WIN_CONVERT_TO_MP3)
        DEMO_MAKE_MEDIA_HELPER(IMFMediaType_SetUINT32(media_type, &MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 320 * 1000 / 8));
#endif
#endif
        return OSWRAPPER_AUDIO_RESULT_SUCCESS;
cleanup:
        IMFMediaType_Release(media_type);
    }

    return OSWRAPPER_AUDIO_RESULT_FAILURE;
}

#define DEMO_WRITE_TO_BUFFER_HELPER_FAIL(X) if (FAILED(X)) { if (media_buffer != NULL) { IMFMediaBuffer_Release(media_buffer); } if (sample != NULL) { IMFSample_Release(sample); } AUDIO_DEMO_CONSOLE_OUTPUT(DEMO_ENC_XSTR(X) " failed!"); return 0; }

static LONGLONG write_buffer_to_writer(IMFSinkWriter* writer, short* buffer, size_t this_iter, size_t frame_size, DWORD audio_stream_index, LONGLONG output_pos, unsigned long sample_rate) {
    /* Create IMFSample */
    IMFSample* sample = NULL;
    IMFMediaBuffer* media_buffer = NULL;
    DEMO_WRITE_TO_BUFFER_HELPER_FAIL(MFCreateSample(&sample));
    DEMO_WRITE_TO_BUFFER_HELPER_FAIL(MFCreateMemoryBuffer(this_iter * frame_size, &media_buffer));
    DEMO_WRITE_TO_BUFFER_HELPER_FAIL(IMFSample_AddBuffer(sample, media_buffer));
    size_t this_multi = this_iter * frame_size;
    DWORD current_length;
    BYTE* sample_audio_data = NULL;
    HRESULT result = IMFMediaBuffer_Lock(media_buffer, &sample_audio_data, &current_length, NULL);
    LONGLONG this_dur = 0;

    if (SUCCEEDED(result)) {
        OSWRAPPER_AUDIO_MEMCPY(sample_audio_data, buffer, this_multi);
        IMFMediaBuffer_Unlock(media_buffer);
        result = IMFMediaBuffer_SetCurrentLength(media_buffer, this_multi);

        if (SUCCEEDED(result)) {
            this_dur = MFllMulDiv(this_iter, 10000000ULL, sample_rate, 0);
            result = IMFSample_SetSampleDuration(sample, this_dur);

            if (FAILED(result)) {
                AUDIO_DEMO_CONSOLE_OUTPUT("IMFSample_SetSampleDuration failed!");
            }

            result = IMFSample_SetSampleTime(sample, output_pos);

            if (FAILED(result)) {
                AUDIO_DEMO_CONSOLE_OUTPUT("IMFSample_SetSampleTime failed!");
            }

            result = IMFSinkWriter_WriteSample(writer, audio_stream_index, sample);

            if (FAILED(result)) {
                AUDIO_DEMO_CONSOLE_OUTPUT("IMFSinkWriter_WriteSample failed!");
            }
        } else {
            AUDIO_DEMO_CONSOLE_OUTPUT("IMFMediaBuffer_SetCurrentLength failed!");
        }
    } else {
        AUDIO_DEMO_CONSOLE_OUTPUT("IMFMediaBuffer_Lock failed!");
    }

    if (sample != NULL) {
        IMFSample_Release(sample);
    }

    if (media_buffer != NULL) {
        IMFMediaBuffer_Release(media_buffer);
    }

    return this_dur;
}

#define DEMO_WIN_ENC__END_FAIL_FALSE(cond) if(!cond) { AUDIO_DEMO_CONSOLE_OUTPUT(DEMO_ENC_XSTR(cond) " failed!"); goto exit; }
#define DEMO_WIN_ENC__END_FAIL(hres) DEMO_WIN_ENC__END_FAIL_FALSE(SUCCEEDED(hres))

/* Decodes and re-encodes a given audio file to WAV */
int main(int argc, char** argv) {
    HRESULT result = CoInitialize(NULL);

    if (FAILED(result)) {
        AUDIO_DEMO_CONSOLE_OUTPUT("CoInitialize failed!");
        return EXIT_FAILURE;
    }

    if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE))) {
        CoUninitialize();
        AUDIO_DEMO_CONSOLE_OUTPUT("MFStartup failed!");
        return EXIT_FAILURE;
    }

    int returnVal = EXIT_FAILURE;
    FILE* output_file = NULL;
    IMFSinkWriter* writer;
    OSWrapper_audio_spec* audio_spec = NULL;
    char* output_path = NULL;
    short* buffer = NULL;
    const char* path = argc < 2 ? "noise.wav" : argv[argc - 1];
    size_t input_string_length = AUDIO_DEMO_STRLEN(path);

    if (!oswrapper_audio_init()) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not initialise oswrapper_audio!");
        goto exit;
    }

    audio_spec = (OSWrapper_audio_spec*) AUDIO_DEMO_CALLOC(1, sizeof(OSWrapper_audio_spec));

    if (audio_spec == NULL) {
        AUDIO_DEMO_CONSOLE_OUTPUT("calloc failed for OSWrapper_audio_spec!");
        goto exit;
    }

    output_path = (char*) OSWRAPPER_AUDIO_MALLOC(input_string_length + sizeof(DEMO_WIN_EXT));

    if (output_path == NULL) {
        AUDIO_DEMO_CONSOLE_OUTPUT("malloc failed for output path!");
        goto exit;
    }

    OSWRAPPER_AUDIO_MEMCPY(output_path, path, input_string_length);
    OSWRAPPER_AUDIO_MEMCPY(output_path + input_string_length, DEMO_WIN_EXT, sizeof(DEMO_WIN_EXT));
    output_file = AUDIO_DEMO_FILE_OPEN_READ(output_path);

    if (output_file != AUDIO_DEMO_INVALID_FILE_TYPE) {
        AUDIO_DEMO_FILE_CLOSE(output_file);
        output_file = AUDIO_DEMO_INVALID_FILE_TYPE;
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Output file %s already exists!\n", output_path);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        goto exit;
    }

    if (make_sink_writer_from_path(output_path, &writer) != OSWRAPPER_AUDIO_RESULT_SUCCESS) {
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Could not create sink writer for output file %s!\n", output_path);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        goto exit;
    }

    audio_spec->sample_rate = SAMPLE_RATE;
    audio_spec->channel_count = CHANNEL_COUNT;
    audio_spec->bits_per_channel = BITS_PER_CHANNEL;
    audio_spec->audio_type = AUDIO_FORMAT;
    audio_spec->endianness_type = ENDIANNESS_TYPE;

    if (oswrapper_audio_load_from_path(path, audio_spec)) {
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Path: %s\nOutput path: %s\nSample rate: %lu\nChannels: %d\nBit depth: %d\n", path, output_path, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);

        if (audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT) {
            AUDIO_DEMO_CONSOLE_OUTPUT("Input format: floating point PCM\n");
        } else {
            AUDIO_DEMO_CONSOLE_OUTPUT("Input format: integer PCM\n");
        }

        if (audio_spec->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG) {
            AUDIO_DEMO_CONSOLE_OUTPUT("Big-endian\n");
        } else {
            AUDIO_DEMO_CONSOLE_OUTPUT("Little-endian\n");
        }

        size_t frame_size = (audio_spec->bits_per_channel / 8) * (audio_spec->channel_count);
        buffer = (short*) AUDIO_DEMO_CALLOC(TEST_PROGRAM_BUFFER_SIZE, frame_size);
        size_t frames_done = 0;
        LONGLONG output_pos = 0;
        DWORD audio_stream_index = 0;
        /* Output stream format */
        IMFMediaType* output_media_type;
        DEMO_WIN_ENC__END_FAIL_FALSE(make_media_type_for_output_format(&output_media_type, audio_spec));
        DEMO_WIN_ENC__END_FAIL(IMFSinkWriter_AddStream(writer, output_media_type, &audio_stream_index));
        IMFMediaType_Release(output_media_type);
        /* Input stream format */
        IMFMediaType* input_media_type;
        DEMO_WIN_ENC__END_FAIL_FALSE(make_media_type_for_input_format(&input_media_type, audio_spec));
        DEMO_WIN_ENC__END_FAIL(IMFSinkWriter_SetInputMediaType(writer, audio_stream_index, input_media_type, NULL));
        IMFMediaType_Release(input_media_type);
        /* Initialise sink writer */
        DEMO_WIN_ENC__END_FAIL(IMFSinkWriter_BeginWriting(writer));

        if (buffer == NULL) {
            AUDIO_DEMO_CONSOLE_OUTPUT("calloc failed for audio decoding buffer!");
            goto audio_cleanup;
        }

        while (1) {
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);

            if (this_iter == 0) {
                break;
            }

            output_pos += write_buffer_to_writer(writer, buffer, this_iter, frame_size, audio_stream_index, output_pos, audio_spec->sample_rate);
            frames_done += this_iter;
        }

        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Encoded " AUDIO_DEMO_FORMAT_SIZE_T " frames of audio, with frame size " AUDIO_DEMO_FORMAT_SIZE_T "\n", frames_done, frame_size);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        returnVal = EXIT_SUCCESS;
audio_cleanup:
        IMFSinkWriter_Finalize(writer);
        IMFSinkWriter_Release(writer);

        if (!oswrapper_audio_free_context(audio_spec)) {
            AUDIO_DEMO_CONSOLE_OUTPUT("Could not free audio context!");
        }
    } else {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not decode audio!");
    }

    if (!oswrapper_audio_uninit()) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not uninitialise oswrapper_audio!");
        returnVal = EXIT_FAILURE;
    }

    if (FAILED(MFShutdown())) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not uninitialise MF!");
        returnVal = EXIT_FAILURE;
    }

exit:
    CoUninitialize();

    if (output_file != AUDIO_DEMO_INVALID_FILE_TYPE) {
        AUDIO_DEMO_FILE_CLOSE(output_file);
        output_file = AUDIO_DEMO_INVALID_FILE_TYPE;
    }

    if (audio_spec != NULL) {
        OSWRAPPER_AUDIO_FREE(audio_spec);
        audio_spec = NULL;
    }

    if (output_path != NULL) {
        OSWRAPPER_AUDIO_FREE(output_path);
        output_path = NULL;
    }

    if (buffer != NULL) {
        OSWRAPPER_AUDIO_FREE(buffer);
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
