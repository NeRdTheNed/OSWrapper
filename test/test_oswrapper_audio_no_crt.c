#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && defined(_VC_NODEFAULTLIB)
/* If we're not using the Windows CRT, use Win32 functions instead */
#define OSWRAPPER_AUDIO_MALLOC(x) HeapAlloc(GetProcessHeap(), 0, x)
#define OSWRAPPER_AUDIO_FREE(x) HeapFree(GetProcessHeap(), 0, x)
/* These are just macros for the C functions, so we have to implement our own versions :/
#define OSWRAPPER_AUDIO_MEMCPY(x, y, amount) CopyMemory(x, y, amount)
#define OSWRAPPER_AUDIO_MEMMOVE(x, y, amount) MoveMemory(x, y, amount)*/
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

/* Not optimised. */
static void* bad_memmove(void* destination, const void* source, size_t num) {
    unsigned char* dest_cast = (unsigned char*) destination;
    unsigned char* source_cast = (unsigned char*) source;

    if (dest_cast < source_cast) {
        /* Forward copy */
        return bad_memcpy(destination, source, num);
    } else {
        /* Backward copy */
        size_t i = num;

        while (i--) {
            dest_cast[i] = source_cast[i];
        }

        return destination;
    }
}
#define OSWRAPPER_AUDIO_MEMCPY(x, y, amount) bad_memcpy(x, y, amount)
#define OSWRAPPER_AUDIO_MEMMOVE(x, y, amount) bad_memmove(x, y, amount)
#endif
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* Include headers for CoInit, and link with audio decoding libraries */
#include <objbase.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

#ifdef _VC_NODEFAULTLIB
/* If we're not using the Windows CRT, use Win32 functions instead */
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
#endif
#endif

#ifndef _VC_NODEFAULTLIB
/* If we are using the Windows CRT or we're not on Windows,
include headers for standard C functions. */
#include <stdio.h>
#include <stdlib.h>
#endif

/* Standard C functions and types. */
#ifndef AUDIO_DEMO_CONSOLE_OUTPUT
#define AUDIO_DEMO_CONSOLE_OUTPUT(x) fputs(x, stdout)
#endif
#ifndef AUDIO_DEMO_STRLEN
#define AUDIO_DEMO_STRLEN(x) strlen(x)
#endif
#ifndef AUDIO_DEMO_SNPRINTF
#define AUDIO_DEMO_SNPRINTF(buffer, len, format, ...) snprintf(buffer, len, format, __VA_ARGS__)
#endif
#ifndef AUDIO_DEMO_FILE_TYPE
#define AUDIO_DEMO_FILE_TYPE FILE*
#endif
#ifndef AUDIO_DEMO_INVALID_FILE_TYPE
#define AUDIO_DEMO_INVALID_FILE_TYPE NULL
#endif
#ifndef AUDIO_DEMO_FILE_NUM_WRITTEN_TYPE
#define AUDIO_DEMO_FILE_NUM_WRITTEN_TYPE size_t
#endif
#ifndef AUDIO_DEMO_FILE_OPEN_READ
#define AUDIO_DEMO_FILE_OPEN_READ(path) fopen(path, "rb")
#endif
#ifndef AUDIO_DEMO_FILE_OPEN_WRITE
#define AUDIO_DEMO_FILE_OPEN_WRITE(path) fopen(path, "wb")
#endif
#ifndef AUDIO_DEMO_FILE_WRITE
#define AUDIO_DEMO_FILE_WRITE(num_written, mem, elem_size, elem, file) num_written = fwrite(mem, elem_size, elem, file)
#endif
#ifndef AUDIO_DEMO_FILE_CLOSE
#define AUDIO_DEMO_FILE_CLOSE(x) fclose(x)
#endif
#ifndef AUDIO_DEMO_CALLOC
#define AUDIO_DEMO_CALLOC(x, size) calloc(x, size)
#endif

#ifndef AUDIO_DEMO_FORMAT_SIZE_T
#define AUDIO_DEMO_FORMAT_SIZE_T "%zu"
#endif

#define AUDIO_DEMO_PRINT_BUFFER_SIZE 1025
static char print_buffer[AUDIO_DEMO_PRINT_BUFFER_SIZE] = "";

/* Windows specific code to allow compiling without the CRT */
#ifdef _VC_NODEFAULTLIB
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
#endif

#ifdef HINT_OUTPUT_FORMAT
#define SAMPLE_RATE 44100
#define CHANNEL_COUNT 2
#define BITS_PER_CHANNEL 16
#else
#define SAMPLE_RATE 0
#define CHANNEL_COUNT 0
#define BITS_PER_CHANNEL 0
#endif

#define TEST_PROGRAM_BUFFER_SIZE 0x50

/* Decodes a given audio file to raw PCM data */
int main(int argc, char** argv) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    HRESULT result = CoInitialize(NULL);

    if (FAILED(result)) {
        AUDIO_DEMO_CONSOLE_OUTPUT("CoInitialize failed!");
        return EXIT_FAILURE;
    }

#endif
    int returnVal = EXIT_FAILURE;
    AUDIO_DEMO_FILE_TYPE output_file = AUDIO_DEMO_INVALID_FILE_TYPE;
    OSWrapper_audio_spec* audio_spec = NULL;
    char* output_path = NULL;
    short* buffer = NULL;

    if (!oswrapper_audio_init()) {
        AUDIO_DEMO_CONSOLE_OUTPUT("Could not initialise oswrapper_audio!");
        goto exit;
    }

    const char* path = argc < 2 ? "noise.wav" : argv[argc - 1];
    audio_spec = (OSWrapper_audio_spec*) AUDIO_DEMO_CALLOC(1, sizeof(OSWrapper_audio_spec));

    if (audio_spec == NULL) {
        AUDIO_DEMO_CONSOLE_OUTPUT("calloc failed for OSWrapper_audio_spec!");
        goto exit;
    }

    size_t input_string_length = AUDIO_DEMO_STRLEN(path);
    output_path = OSWRAPPER_AUDIO_MALLOC(input_string_length + sizeof(".raw"));

    if (output_path == NULL) {
        AUDIO_DEMO_CONSOLE_OUTPUT("malloc failed for output path!");
        goto exit;
    }

    OSWRAPPER_AUDIO_MEMCPY(output_path, path, input_string_length);
    OSWRAPPER_AUDIO_MEMCPY(output_path + input_string_length, ".raw", sizeof(".raw"));
    output_file = AUDIO_DEMO_FILE_OPEN_READ(output_path);

    if (output_file != AUDIO_DEMO_INVALID_FILE_TYPE) {
        AUDIO_DEMO_FILE_CLOSE(output_file);
        output_file = AUDIO_DEMO_INVALID_FILE_TYPE;
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Output file %s already exists!\n", output_path);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        goto exit;
    }

    output_file = AUDIO_DEMO_FILE_OPEN_WRITE(output_path);

    if (output_file == AUDIO_DEMO_INVALID_FILE_TYPE) {
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Output file %s could not be opened for writing!\n", output_path);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        goto exit;
    }

    audio_spec->sample_rate = SAMPLE_RATE;
    audio_spec->channel_count = CHANNEL_COUNT;
    audio_spec->bits_per_channel = BITS_PER_CHANNEL;

    if (oswrapper_audio_load_from_path(path, audio_spec)) {
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Path: %s\nOutput path: %s\nSample rate: %lu\nChannels: %d\nBit depth: %d\n", path, output_path, audio_spec->sample_rate, audio_spec->channel_count, audio_spec->bits_per_channel);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        size_t frame_size = (audio_spec->bits_per_channel / 8) * (audio_spec->channel_count);
        buffer = (short*) AUDIO_DEMO_CALLOC(TEST_PROGRAM_BUFFER_SIZE, frame_size);

        if (buffer == NULL) {
            AUDIO_DEMO_CONSOLE_OUTPUT("calloc failed for audio decoding buffer!");
            goto audio_cleanup;
        }

        size_t frames_done = 0;

        while (1) {
            AUDIO_DEMO_FILE_NUM_WRITTEN_TYPE num_written;
            size_t this_iter = oswrapper_audio_get_samples(audio_spec, buffer, TEST_PROGRAM_BUFFER_SIZE);

            if (this_iter == 0) {
                break;
            }

            AUDIO_DEMO_FILE_WRITE(num_written, buffer, frame_size, this_iter, output_file);
            frames_done += this_iter;
        }

        AUDIO_DEMO_FILE_CLOSE(output_file);
        output_file = AUDIO_DEMO_INVALID_FILE_TYPE;
        AUDIO_DEMO_SNPRINTF(print_buffer, AUDIO_DEMO_PRINT_BUFFER_SIZE, "Decoded " AUDIO_DEMO_FORMAT_SIZE_T " frames of audio, with frame size " AUDIO_DEMO_FORMAT_SIZE_T "\n", frames_done, frame_size);
        AUDIO_DEMO_CONSOLE_OUTPUT(print_buffer);
        returnVal = EXIT_SUCCESS;
audio_cleanup:

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

exit:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    CoUninitialize();
#endif

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
