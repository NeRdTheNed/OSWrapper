/*
This program uses oswrapper_audio to decode and play an audio file twice,
using a modified version of sokol_audio for sound output.

This is a modified version of demo_oswrapper_audio_sokol_audio to allow compiling on Windows
without using a C runtime (abbreviated to CRT).

Usage: demo_oswrapper_audio_sokol_audio_no_crt (audio_file.ext)
If no input is provided, it will play the file named noise.wav in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/demo_oswrapper_audio_sokol_audio_no_crt.c
*/

#ifdef _M_IX86
/* Fix some linking issues with __ftol2 by using a worse version of it */
#define FLOAT_TO_INT FloatToInt
__forceinline int FloatToInt(float f) {
    int i;
    __asm fld f
    __asm fistp i
    return i;
}
#else
#define FLOAT_TO_INT (int)
#endif

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
#endif
#define OSWRAPPER_AUDIO_IMPLEMENTATION
#include "oswrapper_audio.h"

#ifdef __APPLE__
/* Sleep function */
#include <unistd.h>

#define SAUDIO_OSX_USE_SYSTEM_HEADERS
#endif
#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && defined(_VC_NODEFAULTLIB)
#define SOKOL_MALLOC(x) OSWRAPPER_AUDIO_MALLOC(x)
#define SOKOL_FREE(x) OSWRAPPER_AUDIO_FREE(x)
/* MSVC seems to emit references to memcpy even if you don't call it, so we have to redefine it */
#pragma function(memcpy)
void* memcpy(void* destination, const void* source, size_t num) {
    return bad_memcpy(destination, source, num);
}
#define SOKOL_MEMCPY(x, y, amount) bad_memcpy(x, y, amount)
/* MSVC seems to emit references to memset even if you don't call it, so we have to redefine it */
#pragma function(memset)
void* memset(void* destination, int value, size_t amount) {
    size_t i;
    unsigned char* dest_cast = (unsigned char*) destination;

    for (i = 0; i < amount; i++) {
        dest_cast[i] = (unsigned char) value;
    }

    return destination;
}
#define SOKOL_MEMSET(p, val, amount) memset(p, val, amount)
#define SOKOL_ABORT(val, ...) ExitProcess(val)
#define SOKOL_ASSERT
#endif
#define SOKOL_AUDIO_IMPL
#include "sokol_audio_no_crt.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* Include headers for CoInit, and link with audio decoding libraries */
/* Sleep function */
#include <windows.h>
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
#define BITS_PER_CHANNEL 32
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT
#else
#define SAMPLE_RATE 0
#define CHANNEL_COUNT 0
#define BITS_PER_CHANNEL 32
#define AUDIO_FORMAT OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT
#endif

#define FLOAT_BUFFER_SIZE 0x4000
#define SLEEP_TIME 10

#define FAIL_WITH_MESSAGE_ON_COND(cond, message) if ((cond)) { AUDIO_DEMO_CONSOLE_OUTPUT(message); return EXIT_FAILURE; }

int main(int argc, char** argv) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    FAIL_WITH_MESSAGE_ON_COND(FAILED(CoInitialize(NULL)), "CoInitialize failed!");
#endif
    FAIL_WITH_MESSAGE_ON_COND(!oswrapper_audio_init(), "Could not initialise oswrapper_audio!");
    /* Allocate memory for an OSWrapper_audio_spec */
    OSWrapper_audio_spec* audio_spec = (OSWrapper_audio_spec*) AUDIO_DEMO_CALLOC(1, sizeof(OSWrapper_audio_spec));
    FAIL_WITH_MESSAGE_ON_COND((audio_spec == NULL), "calloc failed!");
    /* Hint the desired output format */
    audio_spec->sample_rate = SAMPLE_RATE;
    audio_spec->channel_count = CHANNEL_COUNT;
    audio_spec->bits_per_channel = BITS_PER_CHANNEL;
    audio_spec->audio_type = AUDIO_FORMAT;
    /* Or set these values to zero to use the input format's values.
    The values in audio_spec will always be set to the output format's values
    after initialising an OSWrapper_audio_spec. */
    /* Load audio with desired format */
    int did_load_audio = oswrapper_audio_load_from_path(argc < 2 ? "noise.wav" : argv[argc - 1], audio_spec);
    FAIL_WITH_MESSAGE_ON_COND((did_load_audio == 0), "oswrapper_audio_load_from_path failed!");
    /* audio_spec now contains the output format values. */
    /* sokol_audio: Assert that we're decoding to 32 bit float */
    FAIL_WITH_MESSAGE_ON_COND((audio_spec->audio_type != OSWRAPPER_AUDIO_FORMAT_PCM_FLOAT || audio_spec->bits_per_channel != 32), "Only 32 bit floating point PCM is supported with sokol_audio!");
    /* sokol_audio: Create a suitable config */
#ifdef __cplusplus
    saudio_desc format = { };
#else
    saudio_desc format = { 0 };
#endif
    format.num_channels = audio_spec->channel_count;
    format.sample_rate = audio_spec->sample_rate;
    /* sokol_audio: Initialise sokol_audio */
    saudio_setup(&format);
    FAIL_WITH_MESSAGE_ON_COND((saudio_isvalid() == false), "Failed to initialise sokol_audio!");
    FAIL_WITH_MESSAGE_ON_COND((saudio_sample_rate() != (int) audio_spec->sample_rate), "Output sample rate was not the same as requested sample rate!");
    FAIL_WITH_MESSAGE_ON_COND((saudio_channels() != (int) audio_spec->channel_count), "Output sample rate was not the same as requested sample rate!");
    float float_buffer[FLOAT_BUFFER_SIZE];
    bool first_time = true;
    AUDIO_DEMO_CONSOLE_OUTPUT("Playing sound...");

    /* sokol_audio: Push samples until the file has been decoded twice */
    while (true) {
        const int num_frames = saudio_expect();

        if (num_frames > 0) {
            FAIL_WITH_MESSAGE_ON_COND((saudio_sample_rate() != (int) audio_spec->sample_rate), "Output sample rate was not the same as requested sample rate!");
            FAIL_WITH_MESSAGE_ON_COND((saudio_channels() != (int) audio_spec->channel_count), "Output sample rate was not the same as requested sample rate!");
            const int num_samples = num_frames > (FLOAT_BUFFER_SIZE / saudio_channels()) ? FLOAT_BUFFER_SIZE / saudio_channels() : num_frames;
            size_t decoded_samples = oswrapper_audio_get_samples(audio_spec, (short*) &float_buffer, num_samples);

            if (decoded_samples == 0) {
                if (first_time) {
                    AUDIO_DEMO_CONSOLE_OUTPUT("Playing sound again...");
                    oswrapper_audio_rewind(audio_spec);
                    first_time = false;
                } else {
                    AUDIO_DEMO_CONSOLE_OUTPUT("Finished playing sound!");
                    break;
                }
            } else {
                saudio_push(float_buffer, decoded_samples);
            }
        }

#ifdef _WIN32
        Sleep(SLEEP_TIME);
#elif defined(__APPLE__)
        usleep(SLEEP_TIME * 1000);
#endif
    }

    /* Cleanup */
    saudio_shutdown();
    /* Free audio file */
    int did_close = oswrapper_audio_free_context(audio_spec);
    FAIL_WITH_MESSAGE_ON_COND((!did_close), "Failed to free sound context!");
    FAIL_WITH_MESSAGE_ON_COND(!oswrapper_audio_uninit(), "Could not uninitialise oswrapper_audio!");
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    CoUninitialize();
#endif
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
