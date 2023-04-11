/*
This program uses oswrapper_image to decode an image file,
and display some information about the decoded data.

This is a modified version of test_oswrapper_image to allow compiling on Windows
without using a C runtime (abbreviated to CRT).

Usage: test_oswrapper_image_no_crt (image_file.ext)
If no input is provided, it will decode an embedded version of the file named
face.png in this folder.

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/test/test_oswrapper_image_no_crt.c
*/

#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && defined(_VC_NODEFAULTLIB)
/* If we're not using the Windows CRT, use Win32 functions instead */
#define OSWRAPPER_IMAGE_MALLOC(x) HeapAlloc(GetProcessHeap(), 0, x)
#define OSWRAPPER_IMAGE_FREE(x) HeapFree(GetProcessHeap(), 0, x)
#endif
#define OSWRAPPER_IMAGE_STATIC
#define OSWRAPPER_IMAGE_IMPLEMENTATION
#include "oswrapper_image.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* Include headers for CoInit, and link with image decoding libraries */
#include <objbase.h>
#pragma comment(lib, "windowscodecs.lib")
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
#define IMAGE_DEMO_CONSOLE_OUTPUT(x) WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), x, lstrlen(x), NULL, NULL)
/* HACK: The maximum characters this function writes to a buffer is 1025,
so as long as the buffer is at least 1025 characters, it's "safe" to use. */
#define IMAGE_DEMO_SNPRINTF(buffer, len, format, ...) wsprintfA(buffer, format, __VA_ARGS__)
#endif
#endif

#ifndef _VC_NODEFAULTLIB
/* If we are using the Windows CRT or we're not on Windows,
include headers for standard C functions. */
#include <stdio.h>
#include <stdlib.h>
#endif

/* Standard C functions for printing output, and buffering formatted strings. */
#ifndef IMAGE_DEMO_CONSOLE_OUTPUT
#define IMAGE_DEMO_CONSOLE_OUTPUT(x) fputs(x, stdout)
#endif
#ifndef IMAGE_DEMO_SNPRINTF
#define IMAGE_DEMO_SNPRINTF(buffer, len, format, ...) snprintf(buffer, len, format, __VA_ARGS__)
#endif

unsigned char face_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0x0f, 0xbe, 0x8b, 0x00, 0x00, 0x00,
    0x21, 0x49, 0x44, 0x41, 0x54, 0x08, 0x1d, 0x63, 0x20, 0x06, 0xfc, 0xc7,
    0x8b, 0x41, 0x04, 0x0e, 0x80, 0xaa, 0x00, 0x4a, 0x23, 0xb3, 0x89, 0x37,
    0x01, 0x5d, 0x00, 0x53, 0x01, 0x5e, 0x4c, 0x08, 0x00, 0x00, 0x18, 0x04,
    0x6b, 0x95, 0x57, 0x16, 0xe2, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

unsigned int face_png_len = 90;

#define IMAGE_DEMO_PRINT_BUFFER_SIZE 1025
static char print_buffer[IMAGE_DEMO_PRINT_BUFFER_SIZE] = "";

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
        argv = (char**) OSWRAPPER_IMAGE_MALLOC((argc + 1) * sizeof(char*));

        if (argv != NULL) {
            for (int i = 0; i < argc; i++) {
                /* Get needed size for argument string */
                int wide_length = WideCharToMultiByte(CP_UTF8, 0, win_argv[i], -1, NULL, 0, NULL, NULL);

                if (wide_length > 0) {
                    argv[i] = OSWRAPPER_IMAGE_MALLOC(wide_length * sizeof(char));

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
                OSWRAPPER_IMAGE_FREE(argv[i]);
            }
        }
    }

    /* Free argv */
    if (argv != NULL && argv != argv_error) {
        OSWRAPPER_IMAGE_FREE(argv);
    }

    ExitProcess(return_val);
}
#endif

int main(int argc, char** argv) {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    HRESULT result = CoInitialize(NULL);

    if (FAILED(result)) {
        IMAGE_DEMO_CONSOLE_OUTPUT("CoInitialize failed!");
        return EXIT_FAILURE;
    }

#endif
    int returnVal = EXIT_FAILURE;
    const char* path = argc < 2 ? NULL : argv[argc - 1];

    if (!oswrapper_image_init()) {
        IMAGE_DEMO_CONSOLE_OUTPUT("Could not initialise oswrapper_image!");
        goto exit;
    }

    int width, height, channels;
    unsigned char* image_data;

    if (path == NULL) {
        path = "(internal memory)";
        image_data = oswrapper_image_load_from_memory(face_png, face_png_len, &width, &height, &channels);
    } else {
        image_data = oswrapper_image_load_from_path(path, &width, &height, &channels);
    }

    if (image_data != NULL) {
        oswrapper_image_free(image_data);
        IMAGE_DEMO_SNPRINTF(print_buffer, IMAGE_DEMO_PRINT_BUFFER_SIZE, "Path: %s\nWidth: %d\nHeight: %d\nChannels: %d\n", path, width, height, channels);
        IMAGE_DEMO_CONSOLE_OUTPUT(print_buffer);
        returnVal = EXIT_SUCCESS;
    } else {
        IMAGE_DEMO_CONSOLE_OUTPUT("Could not decode image!");
    }

    if (!oswrapper_image_uninit()) {
        IMAGE_DEMO_CONSOLE_OUTPUT("Could not uninitialise oswrapper_image!");
        returnVal = EXIT_FAILURE;
    }

exit:
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    CoUninitialize();
#endif
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
