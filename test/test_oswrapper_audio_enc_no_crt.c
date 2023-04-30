/*
This program uses oswrapper_audio to decode an audio file,
and encodes the decoded PCM data to a variety of file types using oswrapper_audio_enc.

This is a modified version of test_oswrapper_audio_enc to allow compiling on Windows
without using a C runtime (abbreviated to CRT).

Usage: test_oswrapper_audio_enc_no_crt (audio_file.ext) (optional wanted file format)
If no input is provided, it will decode and encode the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_audio_enc_no_crt.c
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

#define OSWRAPPER_AUDIO_ENC_MALLOC(x) HeapAlloc(GetProcessHeap(), 0, x)
#define OSWRAPPER_AUDIO_ENC_FREE(x) HeapFree(GetProcessHeap(), 0, x)
#define OSWRAPPER_AUDIO_ENC_MEMCPY(x, y, amount) bad_memcpy(x, y, amount)
#define OSWRAPPER_AUDIO_ENC_STATIC
#define OSWRAPPER_AUDIO_ENC_IMPLEMENTATION
#include "oswrapper_audio_enc.h"

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

#define TEST_PROGRAM_BUFFER_SIZE 0x50

static int contains_subs(const char* string, const char* subs) {
    while (*string != '\0') {
        if (*string != *subs) {
            goto nextLoop;
        }

        {
            const char* search = string;
            const char* search_subs = subs;

            while (*search == *search_subs) {
                ++search;
                ++search_subs;
            }

            if (*search_subs == '\0') {
                return 1;
            }
        }

nextLoop:
        ++string;
    }

    return 0;
}

static OSWrapper_audio_enc_output_type demo_get_enum_for_str(const char* type) {
    if (contains_subs(type, "alac") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC;
    } else if (contains_subs(type, "flac") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC;
    } else if (contains_subs(type, "wav") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV;
    } else if (contains_subs(type, "snd") == 0 || contains_subs(type, "au") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND;
    } else if (contains_subs(type, "m4a") == 0 || contains_subs(type, "aac") == 0) {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC;
    } else {
        return OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV;
    }
}

static const char* demo_get_ext_for_enum(OSWrapper_audio_enc_output_type type) {
    switch (type) {
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_SND:
        return ".snd";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_AAC:
    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_ALAC:
        return ".m4a";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_FLAC:
        return ".flac";

    case OSWRAPPER_AUDIO_ENC_OUPUT_FORMAT_WAV:
    default:
        return ".wav";
    }
}

/* Decodes and re-encodes a given audio file to a variety of file types */
int main(int argc, char** argv) {
    HRESULT result = CoInitialize(NULL);

    if (FAILED(result)) {
        AUDIO_DEMO_CONSOLE_OUTPUT("CoInitialize failed!");
        return EXIT_FAILURE;
    }

    int returnVal = EXIT_FAILURE;
    FILE* output_file = NULL;
    OSWrapper_audio_spec* audio_spec = NULL;
    OSWrapper_audio_enc_spec* audio_enc_spec = NULL;
    char* output_path = NULL;
    short* buffer = NULL;
    const char* path;
    const char* type;
    const char* ext = NULL;

    if (argc > 2) {
        path = argv[argc - 2];
        type = argv[argc - 1];
    } else {
        path = argc < 2 ? "noise.wav" : argv[argc - 1];
        type = "m4a";
    }

    OSWrapper_audio_enc_output_type output_type = demo_get_enum_for_str(type);
    ext = demo_get_ext_for_enum(output_type);
    size_t input_string_length = AUDIO_DEMO_STRLEN(path);
    size_t output_ext_length = AUDIO_DEMO_STRLEN(ext) + 1;

    if (!oswrapper_audio_init()) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not initialise oswrapper_audio!");
        goto exit;
    }

    if (!oswrapper_audio_enc_init()) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not initialise oswrapper_audio_enc!");
        goto exit;
    }

    audio_spec = (OSWrapper_audio_spec*) AUDIO_DEMO_CALLOC(1, sizeof(OSWrapper_audio_spec));

    if (audio_spec == NULL) {
        AUDIO_DEMO_CONSOLE_OUTPUT("calloc failed for OSWrapper_audio_spec!");
        goto exit;
    }

    audio_enc_spec = (OSWrapper_audio_enc_spec*) AUDIO_DEMO_CALLOC(1, sizeof(OSWrapper_audio_enc_spec));

    if (audio_enc_spec == NULL) {
        AUDIO_DEMO_CONSOLE_OUTPUT("calloc failed for OSWrapper_audio_enc_spec!");
        goto exit;
    }

    output_path = (char*) OSWRAPPER_AUDIO_MALLOC(input_string_length + output_ext_length);

    if (output_path == NULL) {
        AUDIO_DEMO_CONSOLE_OUTPUT("malloc failed for output path!");
        goto exit;
    }

    OSWRAPPER_AUDIO_MEMCPY(output_path, path, input_string_length);
    OSWRAPPER_AUDIO_MEMCPY(output_path + input_string_length, ext, output_ext_length);
    output_file = AUDIO_DEMO_FILE_OPEN_READ(output_path);

    if (output_file != AUDIO_DEMO_INVALID_FILE_TYPE) {
        AUDIO_DEMO_FILE_CLOSE(output_file);
        output_file = AUDIO_DEMO_INVALID_FILE_TYPE;
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Output file %s already exists!\n", output_path);
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
            AUDIO_DEMO_CONSOLE_OUTPUT("Output format: floating point PCM\n");
        } else {
            AUDIO_DEMO_CONSOLE_OUTPUT("Output format: integer PCM\n");
        }

        if (audio_spec->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG) {
            AUDIO_DEMO_CONSOLE_OUTPUT("Big-endian\n");
        } else {
            AUDIO_DEMO_CONSOLE_OUTPUT("Little-endian\n");
        }

        size_t frame_size = (audio_spec->bits_per_channel / 8) * (audio_spec->channel_count);
        buffer = (short*) AUDIO_DEMO_CALLOC(TEST_PROGRAM_BUFFER_SIZE, frame_size);
        size_t frames_done = 0;
        audio_enc_spec->sample_rate = audio_spec->sample_rate;
        audio_enc_spec->channel_count = audio_spec->channel_count;
        audio_enc_spec->bits_per_channel = audio_spec->bits_per_channel;
        audio_enc_spec->input_pcm_type = audio_spec->audio_type == OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT ? OSWRAPPER_AUDIO_ENC_PCM_FLOAT : OSWRAPPER_AUDIO_ENC_PCM_INTEGER;
        audio_enc_spec->input_pcm_endianness_type = audio_spec->endianness_type == OSWRAPPER_AUDIO_ENDIANNESS_BIG ? OSWRAPPER_AUDIO_ENC_ENDIANNESS_BIG : OSWRAPPER_AUDIO_ENC_ENDIANNESS_LITTLE;
        audio_enc_spec->output_type = output_type;

        if (!oswrapper_audio_enc_make_file_from_path(output_path, audio_enc_spec)) {
            AUDIO_DEMO_CONSOLE_OUTPUT("Could not encode audio!");
            goto dec_cleanup;
        }

        if (buffer == NULL) {
            AUDIO_DEMO_CONSOLE_OUTPUT("calloc failed for audio decoding buffer!");
            goto audio_cleanup;
        }

        while (1) {
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);

            if (this_iter == 0) {
                break;
            }

            if (!oswrapper_audio_enc_encode_samples(audio_enc_spec, buffer, this_iter)) {
                AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Error writing frame at position " AUDIO_DEMO_FORMAT_SIZE_T "!\n", frames_done);
                AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
            }

            frames_done += this_iter;
        }

        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Encoded " AUDIO_DEMO_FORMAT_SIZE_T " frames of audio, with frame size " AUDIO_DEMO_FORMAT_SIZE_T "\n", frames_done, frame_size);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        returnVal = EXIT_SUCCESS;
audio_cleanup:

        if (!oswrapper_audio_enc_finalise_file_context(audio_enc_spec)) {
            AUDIO_DEMO_CONSOLE_OUTPUT("Could not free audio encoding context!");
        }

dec_cleanup:

        if (!oswrapper_audio_free_context(audio_spec)) {
            AUDIO_DEMO_CONSOLE_OUTPUT("Could not free audio decoding context!");
        }
    } else {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not decode audio!");
    }

    if (!oswrapper_audio_enc_uninit()) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not uninitialise oswrapper_audio_enc!");
        returnVal = EXIT_FAILURE;
    }

    if (!oswrapper_audio_uninit()) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not uninitialise oswrapper_audio!");
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

    if (audio_enc_spec != NULL) {
        OSWRAPPER_AUDIO_FREE(audio_enc_spec);
        audio_enc_spec = NULL;
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
