/*
OSWrapper image: Load images with the built in OS image decoders.

Usage:

OSWrapper image has 2 ways of loading an image: from a path, or from memory.
In both cases, you also pass pointers to 3 integers, which get set to the
width, height, and channels of the decoded image.

Load an image from memory:

unsigned char* file_data = ...;
int file_length = ...;
int width, height, channels;

unsigned char* image_data = oswrapper_image_load_from_memory(file_data, file_length, &width, &height, &channels);
if (image_data != NULL) {
  (do something with image_data)
  oswrapper_image_free(image_data);
}

Load an image from a path to a file:

const char* path = "some/path/image.png";
int width, height, channels;

unsigned char* image_data = oswrapper_image_load_from_path(path, &width, &height, &channels);
if (image_data != NULL) {
  (do something with image_data)
  oswrapper_image_free(image_data);
}

Make sure to call oswrapper_image_init() before using the library.
Call oswrapper_image_uninit() after you no longer need to use oswrapper_image.

Platform requirements:
- On macOS, link with AppKit
- On Windows, call CoInitialize before using the library, and link with windowscodecs.lib
- On Emscripten platforms, compile with Asyncify

The latest version of this file can be found at
https://github.com/NeRdTheNed/OSWrapper/blob/main/oswrapper_image.h
*/

#ifndef OSWRAPPER_INCLUDE_OSWRAPPER_IMAGE_H
#define OSWRAPPER_INCLUDE_OSWRAPPER_IMAGE_H
#ifndef OSWRAPPER_IMAGE_DEF
#ifdef OSWRAPPER_IMAGE_STATIC
#define OSWRAPPER_IMAGE_DEF static
#else
#define OSWRAPPER_IMAGE_DEF extern
#endif
#endif /* OSWRAPPER_IMAGE_DEF */

/* You can make these functions return actual booleans if you want */
#ifndef OSWRAPPER_IMAGE_RESULT_TYPE
#define OSWRAPPER_IMAGE_RESULT_TYPE int
#endif

#ifndef OSWRAPPER_IMAGE_RESULT_SUCCESS
#define OSWRAPPER_IMAGE_RESULT_SUCCESS 1
#endif
#ifndef OSWRAPPER_IMAGE_RESULT_FAILURE
#define OSWRAPPER_IMAGE_RESULT_FAILURE 0
#endif

/* Call oswrapper_image_init() before using the library,
and call oswrapper_image_uninit() after you're done using the library. */
OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_init(void);
OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_uninit(void);

#ifdef OSWRAPPER_IMAGE_EXPERIMENTAL
/* Unstable-ish API */
typedef struct OSWrapper_image_decoded_data {
    void* internal_data;
    unsigned char* image_data;
} OSWrapper_image_decoded_data;

OSWRAPPER_IMAGE_DEF void oswrapper_image_free_nocopy(OSWrapper_image_decoded_data* decoded_data);
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_memory_nocopy(unsigned char* image, int length, int* width, int* height, int* channels);
#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_path_nocopy(const char* path, int* width, int* height, int* channels);
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */
#endif

/* Stable-ish API */
OSWRAPPER_IMAGE_DEF void oswrapper_image_free(unsigned char* image_data);
OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_memory(unsigned char* image, int length, int* width, int* height, int* channels);
#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_path(const char* path, int* width, int* height, int* channels);
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_IMAGE_IMPLEMENTATION
#ifndef OSWRAPPER_IMAGE_NO_INCLUDE_STDLIB
#include <stdlib.h>
#endif

#ifndef OSWRAPPER_IMAGE_MALLOC
#define OSWRAPPER_IMAGE_MALLOC(x) malloc(x)
#endif /* OSWRAPPER_IMAGE_MALLOC */
#ifndef OSWRAPPER_IMAGE_FREE
#define OSWRAPPER_IMAGE_FREE(x) free(x)
#endif /* OSWRAPPER_IMAGE_FREE */

#ifdef __APPLE__
#if !defined(OSWRAPPER_IMAGE_USE_MAC_OBJC_IMPL) && !defined(OSWRAPPER_IMAGE_NO_USE_MAC_OBJC_IMPL)
#define OSWRAPPER_IMAGE_USE_MAC_OBJC_IMPL
#endif /* !defined(OSWRAPPER_IMAGE_USE_MAC_OBJC_IMPL) && !defined(OSWRAPPER_IMAGE_NO_USE_MAC_OBJC_IMPL) */
#endif /* __APPLE__ */

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#if !defined(OSWRAPPER_IMAGE_USE_WIN_IMCOM_IMPL) && !defined(OSWRAPPER_IMAGE_NO_USE_WIN_IMCOM_IMPL)
#define OSWRAPPER_IMAGE_USE_WIN_IMCOM_IMPL
#endif /* !defined(OSWRAPPER_IMAGE_USE_WIN_IMCOM_IMPL) && !defined(OSWRAPPER_IMAGE_NO_USE_WIN_IMCOM_IMPL) */
#endif

#ifdef EMSCRIPTEN
#if !defined(OSWRAPPER_IMAGE_USE_EMSCRIPTEN_PRELOAD_IMPL) && !defined(OSWRAPPER_IMAGE_USE_EMSCRIPTEN_PRELOAD_IMPL)
#define OSWRAPPER_IMAGE_USE_EMSCRIPTEN_PRELOAD_IMPL
#endif /* !defined(OSWRAPPER_IMAGE_USE_EMSCRIPTEN_PRELOAD_IMPL) && !defined(OSWRAPPER_IMAGE_USE_EMSCRIPTEN_PRELOAD_IMPL) */
#endif

#ifdef OSWRAPPER_IMAGE_USE_MAC_OBJC_IMPL
/* Start macOS implementation */
#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc-runtime.h>

#ifndef NSINTEGER_DEFINED
#define NSINTEGER_DEFINED 1

#if (defined(__LP64__) && __LP64__) || (defined(NS_BUILD_32_LIKE_64) && NS_BUILD_32_LIKE_64)
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else
typedef int NSInteger;
typedef unsigned int NSUInteger;
#endif

#define NSIntegerMax LONG_MAX
#define NSIntegerMin LONG_MIN
#define NSUIntegerMax ULONG_MAX

#endif /* NSINTEGER_DEFINED */

/* This is mostly just to allow compiling with Apple GCC 4.2 and below. */
#if !(defined(__ppc64__) || defined(__ppc__)) || defined(MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
/* New msgSend prototype */
#ifdef OBJC_OLD_DISPATCH_PROTOTYPES
#undef OBJC_OLD_DISPATCH_PROTOTYPES
#define OBJC_OLD_DISPATCH_PROTOTYPES 0
#endif /* OBJC_OLD_DISPATCH_PROTOTYPES */
#define oswrapper__objc_msgSend_t(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))objc_msgSend)
#else
/* Old msgSend prototype */
#define oswrapper__objc_msgSend_t(RET, ...) (RET)objc_msgSend
#endif /* defined(MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8 */

#if defined(MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
extern id objc_alloc(Class class);
#define oswrapper__objc_alloc(class) objc_alloc(class)
#else
#define oswrapper__objc_alloc(class) oswrapper__objc_msgSend_t(id)((id) class, sel_registerName("alloc"))
#endif

#define oswrapper__objc_init(x) oswrapper__objc_msgSend_t(id)(x, sel_registerName("init"))

#if defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
extern id objc_alloc_init(Class class);
#define oswrapper__objc_alloc_init(class) objc_alloc_init(class)
#else
#define oswrapper__objc_alloc_init(class) oswrapper__objc_init(oswrapper__objc_alloc(class))
#endif

#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
extern void objc_autoreleasePoolPop(void *pool);
extern void *objc_autoreleasePoolPush(void);
#define OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(pool) objc_autoreleasePoolPop(pool)
#define OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_PUSH() objc_autoreleasePoolPush()
#else
#define OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(pool) oswrapper__objc_msgSend_t(void)(pool, sel_registerName("drain"))
#define OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_PUSH() oswrapper__objc_alloc_init(objc_getClass("NSAutoreleasePool"))
#endif

static id oswrapper__setup_image_from_memory(id imageData, int* width, int* height, int* channels) {
    id bitmap = oswrapper__objc_alloc(objc_getClass("NSBitmapImageRep"));
    bitmap = oswrapper__objc_msgSend_t(id, id)(bitmap, sel_registerName("initWithData:"), imageData);

    if (bitmap == nil) {
        return nil;
    }

    *width = (int) oswrapper__objc_msgSend_t(NSInteger)(bitmap, sel_registerName("pixelsWide"));
    *height = (int) oswrapper__objc_msgSend_t(NSInteger)(bitmap, sel_registerName("pixelsHigh"));
    *channels = (int) oswrapper__objc_msgSend_t(NSInteger)(bitmap, sel_registerName("bitsPerPixel")) / 8;
    return bitmap;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
static id oswrapper__setup_image_from_path(const char* path, int* width, int* height, int* channels) {
    id image_path = oswrapper__objc_alloc(objc_getClass("NSString"));
    image_path = oswrapper__objc_msgSend_t(id, const char*)(image_path, sel_registerName("initWithUTF8String:"), path);
    id image_data = oswrapper__objc_alloc(objc_getClass("NSData"));
    image_data = oswrapper__objc_msgSend_t(id, id)(image_data, sel_registerName("initWithContentsOfFile:"), image_path);
    oswrapper__objc_msgSend_t(void)(image_path, sel_registerName("release"));
    id bitmap = oswrapper__setup_image_from_memory(image_data, width, height, channels);
    oswrapper__objc_msgSend_t(void)(image_data, sel_registerName("release"));
    return bitmap;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */

static unsigned char* oswrapper__create_decoded_data_copied(id bitmap, size_t dataSize) {
    unsigned char* bitmap_data = oswrapper__objc_msgSend_t(unsigned char*)(bitmap, sel_registerName("bitmapData"));
    unsigned char* decoded_data = (unsigned char*) OSWRAPPER_IMAGE_MALLOC(dataSize);

    if (decoded_data != NULL) {
        memcpy(decoded_data, bitmap_data, dataSize);
    }

    oswrapper__objc_msgSend_t(void)(bitmap, sel_registerName("release"));
    return decoded_data;
}

OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_init(void) {
    return OSWRAPPER_IMAGE_RESULT_SUCCESS;
}

OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_uninit(void) {
    return OSWRAPPER_IMAGE_RESULT_SUCCESS;
}

#ifdef OSWRAPPER_IMAGE_EXPERIMENTAL
static OSWrapper_image_decoded_data* oswrapper__create_decoded_data(id bitmap) {
    unsigned char* bitmap_data = oswrapper__objc_msgSend_t(unsigned char*)(bitmap, sel_registerName("bitmapData"));
    OSWrapper_image_decoded_data* decoded_data = (OSWrapper_image_decoded_data*) OSWRAPPER_IMAGE_MALLOC(sizeof(OSWrapper_image_decoded_data));

    if (decoded_data != NULL) {
        decoded_data->internal_data = (void*) bitmap;
        decoded_data->image_data = bitmap_data;
        return decoded_data;
    }

    oswrapper__objc_msgSend_t(void)(bitmap, sel_registerName("release"));
    return NULL;
}

OSWRAPPER_IMAGE_DEF void oswrapper_image_free_nocopy(OSWrapper_image_decoded_data* decoded_data) {
    if (decoded_data != NULL) {
        id bitmap = (id) decoded_data->internal_data;

        if (bitmap != nil) {
            oswrapper__objc_msgSend_t(void)(bitmap, sel_registerName("release"));
            decoded_data->internal_data = nil;
        }

        OSWRAPPER_IMAGE_FREE(decoded_data);
    }
}

OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_memory_nocopy(unsigned char* image, int length, int* width, int* height, int* channels) {
    void* autorelease_pool = OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_PUSH();
    CFDataRef image_data_cf = CFDataCreateWithBytesNoCopy(NULL, image, length, kCFAllocatorNull);
    id bitmap = oswrapper__setup_image_from_memory((id) image_data_cf, width, height, channels);
    CFRelease(image_data_cf);

    if (bitmap == nil) {
        OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
        return NULL;
    }

    OSWrapper_image_decoded_data* decoded_data = oswrapper__create_decoded_data(bitmap);
    OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
    return decoded_data;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_path_nocopy(const char* path, int* width, int* height, int* channels) {
    void* autorelease_pool = OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_PUSH();
    id bitmap = oswrapper__setup_image_from_path(path, width, height, channels);

    if (bitmap == nil) {
        OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
        return NULL;
    }

    OSWrapper_image_decoded_data* decoded_data = oswrapper__create_decoded_data(bitmap);
    OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
    return decoded_data;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */
#endif

OSWRAPPER_IMAGE_DEF void oswrapper_image_free(unsigned char* image_data) {
    if (image_data != NULL) {
        OSWRAPPER_IMAGE_FREE(image_data);
    }
}

OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_memory(unsigned char* image, int length, int* width, int* height, int* channels) {
    void* autorelease_pool = OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_PUSH();
    CFDataRef image_data_cf = CFDataCreateWithBytesNoCopy(NULL, image, length, kCFAllocatorNull);
    id bitmap = oswrapper__setup_image_from_memory((id) image_data_cf, width, height, channels);
    CFRelease(image_data_cf);

    if (bitmap == nil) {
        OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
        return NULL;
    }

    size_t dataSize = ((size_t) * width) * ((size_t) * height) * ((size_t) * channels);
    unsigned char* decoded_data = oswrapper__create_decoded_data_copied(bitmap, dataSize);
    OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
    return decoded_data;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_path(const char* path, int* width, int* height, int* channels) {
    void* autorelease_pool = OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_PUSH();
    id bitmap = oswrapper__setup_image_from_path(path, width, height, channels);

    if (bitmap == nil) {
        OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
        return NULL;
    }

    size_t dataSize = ((size_t) * width) * ((size_t) * height) * ((size_t) * channels);
    unsigned char* decoded_data = oswrapper__create_decoded_data_copied(bitmap, dataSize);
    OSWRAPPER_IMAGE__OBJC_AUTORELEASE_POOL_POP(autorelease_pool);
    return decoded_data;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */
/* End macOS implementation */
#elif defined(OSWRAPPER_IMAGE_USE_WIN_IMCOM_IMPL)
/* Start Win32 WIC implementation */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include "wincodec.h"

#if defined(WINCODEC_SDK_VERSION2) && WINCODEC_SDK_VERSION >= WINCODEC_SDK_VERSION2
#define OSWRAPPER_IMAGE__USING_WINCODEC_SDK_VERSION2
#define OSWRAPPER_IMAGE__FIRST_CLSID CLSID_WICImagingFactory2
#define OSWRAPPER_IMAGE__FIRST_IID IID_IWICImagingFactory2
#else
#define OSWRAPPER_IMAGE__FIRST_CLSID CLSID_WICImagingFactory
#define OSWRAPPER_IMAGE__FIRST_IID IID_IWICImagingFactory
#endif

static IWICImagingFactory* oswrapper_image__factory;

OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_init(void) {
    HRESULT result;

    if (oswrapper_image__factory != NULL) {
        return OSWRAPPER_IMAGE_RESULT_SUCCESS;
    }

    result = CoCreateInstance(&OSWRAPPER_IMAGE__FIRST_CLSID, NULL, CLSCTX_INPROC_SERVER, &OSWRAPPER_IMAGE__FIRST_IID, &oswrapper_image__factory);

    if (SUCCEEDED(result)) {
        return OSWRAPPER_IMAGE_RESULT_SUCCESS;
    } else {
#ifdef OSWRAPPER_IMAGE__USING_WINCODEC_SDK_VERSION2
        result = CoCreateInstance(&CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, &oswrapper_image__factory);

        if (SUCCEEDED(result)) {
            return OSWRAPPER_IMAGE_RESULT_SUCCESS;
        }

#endif
    }

    return OSWRAPPER_IMAGE_RESULT_FAILURE;
}

OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_uninit(void) {
    if (oswrapper_image__factory != NULL) {
        IWICImagingFactory_Release(oswrapper_image__factory);
    }

    return OSWRAPPER_IMAGE_RESULT_SUCCESS;
}

OSWRAPPER_IMAGE_DEF void oswrapper_image_free(unsigned char* image_data) {
    if (image_data != NULL) {
        OSWRAPPER_IMAGE_FREE(image_data);
    }
}

static unsigned char* oswrapper__setup_image_from_decoder(IWICBitmapDecoder* decoder, int* width, int* height, int* channels) {
    IWICBitmapFrameDecode* frame;
    unsigned char* decoded_data;
    HRESULT result = IWICBitmapDecoder_GetFrame(decoder, 0, &frame);
    decoded_data = NULL;

    if (SUCCEEDED(result)) {
        IWICFormatConverter* converter;
        result = IWICImagingFactory_CreateFormatConverter(oswrapper_image__factory, &converter);

        if (SUCCEEDED(result)) {
            /* TODO Get original format / bit depth */
            result = IWICFormatConverter_Initialize(converter, (IWICBitmapSource*) frame, &GUID_WICPixelFormat32bppPRGBA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

            if (SUCCEEDED(result)) {
                result = IWICBitmapFrameDecode_GetSize(frame, width, height);

                if (SUCCEEDED(result)) {
                    *channels = 4;
                    {
                        size_t data_size = ((size_t) * width) * ((size_t) * height) * ((size_t) * channels);
                        decoded_data = (unsigned char*) OSWRAPPER_IMAGE_MALLOC(data_size);

                        if (decoded_data != NULL) {
                            IWICFormatConverter_CopyPixels(converter, NULL, (*width) * (*channels), data_size, decoded_data);
                        }
                    }
                }
            }

            IWICFormatConverter_Release(converter);
        }

        IWICBitmapFrameDecode_Release(frame);
    }

    IWICBitmapDecoder_Release(decoder);
    return decoded_data;
}

OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_memory(unsigned char* image, int length, int* width, int* height, int* channels) {
    unsigned char* decoded_data = NULL;

    if (oswrapper_image__factory != NULL) {
        IWICStream* stream;
        HRESULT result = IWICImagingFactory_CreateStream(oswrapper_image__factory, &stream);

        if (SUCCEEDED(result)) {
            result = IWICStream_InitializeFromMemory(stream, image, length);

            if (SUCCEEDED(result)) {
                IWICBitmapDecoder* decoder;
                result = IWICImagingFactory_CreateDecoderFromStream(oswrapper_image__factory, (IStream*) stream, NULL, WICDecodeMetadataCacheOnDemand, &decoder);

                if (SUCCEEDED(result)) {
                    decoded_data = oswrapper__setup_image_from_decoder(decoder, width, height, channels);
                }
            }

            IWICStream_Release(stream);
        }
    }

    return decoded_data;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_path(const char* path, int* width, int* height, int* channels) {
    unsigned char* decoded_data = NULL;

    if (oswrapper_image__factory != NULL) {
        /* TODO Handle string conversion */
        HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (file != INVALID_HANDLE_VALUE) {
            IWICBitmapDecoder* decoder;
            HRESULT result = IWICImagingFactory_CreateDecoderFromFileHandle(oswrapper_image__factory, (ULONG_PTR) file, NULL, WICDecodeMetadataCacheOnDemand, &decoder);

            if (SUCCEEDED(result)) {
                decoded_data = oswrapper__setup_image_from_decoder(decoder, width, height, channels);
            }

            CloseHandle(file);
        }
    }

    return decoded_data;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_IMAGE_EXPERIMENTAL
/* TODO This actually copies more than the alternative */
OSWRAPPER_IMAGE_DEF void oswrapper_image_free_nocopy(OSWrapper_image_decoded_data* decoded_data) {
    if (decoded_data != NULL) {
        if (decoded_data->image_data != NULL) {
            oswrapper_image_free(decoded_data->image_data);
        }

        OSWRAPPER_IMAGE_FREE(decoded_data);
    }
}

/* TODO This actually copies more than the alternative */
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_memory_nocopy(unsigned char* image, int length, int* width, int* height, int* channels) {
    unsigned char* image_data = oswrapper_image_load_from_memory(image, length, width, height, channels);

    if (image_data != NULL) {
        OSWrapper_image_decoded_data* decoded_data = (OSWrapper_image_decoded_data*) OSWRAPPER_IMAGE_MALLOC(sizeof(OSWrapper_image_decoded_data));

        if (decoded_data != NULL) {
            decoded_data->internal_data = NULL;
            decoded_data->image_data = image_data;
            return decoded_data;
        }
    }

    return NULL;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
/* TODO This actually copies more than the alternative */
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_path_nocopy(const char* path, int* width, int* height, int* channels) {
    unsigned char* image_data = oswrapper_image_load_from_path(path, width, height, channels);

    if (image_data != NULL) {
        OSWrapper_image_decoded_data* decoded_data = (OSWrapper_image_decoded_data*) OSWRAPPER_IMAGE_MALLOC(sizeof(OSWrapper_image_decoded_data));

        if (decoded_data != NULL) {
            decoded_data->internal_data = NULL;
            decoded_data->image_data = image_data;
            return decoded_data;
        }
    }

    return NULL;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */
#endif
/* End Win32 WIC implementation */
#elif defined(OSWRAPPER_IMAGE_USE_EMSCRIPTEN_PRELOAD_IMPL)
/* TODO: Requires Asyncify, multithreading isn't finished. */
/* Start Emscripten preloader implementation */
#include <emscripten.h>
#include <emscripten/atomic.h>

/* TODO Unfinished multithreading */
static volatile uint32_t oswrapper_image__preload_janky_lock = 0;

static unsigned char* oswrapper_image__load_from_path_post_preload(const char* path, int* width, int* height, int* channels) {
    unsigned char* try_get_preloaded_data = (unsigned char*) emscripten_get_preloaded_image_data(path, width, height);

    if (try_get_preloaded_data != NULL) {
        /* TODO Currently only returns 4 channel decoded data, API doesn't seem to mention this? */
        *channels = 4;
        return try_get_preloaded_data;
    }

    return NULL;
}

OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_init(void) {
    return OSWRAPPER_IMAGE_RESULT_SUCCESS;
}

OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_uninit(void) {
    return OSWRAPPER_IMAGE_RESULT_SUCCESS;
}

OSWRAPPER_IMAGE_DEF void oswrapper_image_free(unsigned char* image_data) {
    if (image_data != NULL) {
        free(image_data);
    }
}

static void oswrapper_image__load_mem_onload(void* arg, const char* fakename) {
    const char** current_fakename = (const char**) arg;
    /* TODO Unfinished multithreading */
    /* Notify that a callback has been called */
    *current_fakename = fakename;
    emscripten_atomic_store_u32((void*) &oswrapper_image__preload_janky_lock, 0);
}

static void oswrapper_image__load_mem_onerror(void* arg) {
    (void) arg;
    /* TODO Unfinished multithreading */
    /* Notify that a callback has been called */
    emscripten_atomic_store_u32((void*) &oswrapper_image__preload_janky_lock, 0);
}

OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_memory(unsigned char* image, int length, int* width, int* height, int* channels) {
    volatile uint32_t janky_lock_val;
    const char* current_fakename = NULL;

    /* TODO Unfinished multithreading */
    /* Busy acquire the lock */
    do {
        janky_lock_val = emscripten_atomic_cas_u32((void*) &oswrapper_image__preload_janky_lock, 0, 1);
    } while (janky_lock_val);

    emscripten_run_preload_plugins_data((char*) image, length, "png", (void*) &current_fakename, oswrapper_image__load_mem_onload, oswrapper_image__load_mem_onerror);

    /* TODO Unfinished multithreading */
    /* Busy wait until the callbacks are called */
    do {
        /* TODO emscripten_run_preload_plugins uses the main thread to load images, which we are probably on.
        Busy waiting will therefore normally deadlock,
        because the image load callbacks will never run without returning to the main thread.
        Loading synchronously can only work if we're using Asyncify,
        because calling emscripten_sleep will allow the main thread to run the preload plugins. */
        emscripten_sleep(100);
        janky_lock_val = emscripten_atomic_load_u32((void*) &oswrapper_image__preload_janky_lock);
    } while (janky_lock_val == 1);

    if (current_fakename != NULL) {
        unsigned char* try_get_preloaded_data = oswrapper_image__load_from_path_post_preload(current_fakename, width, height, channels);
        free((void*) current_fakename);
        return try_get_preloaded_data;
    }

    return NULL;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
static void oswrapper_image__load_path_onload(const char* textureName) {
    (void) textureName;
    /* TODO Unfinished multithreading */
    /* Notify that a callback has been called */
    emscripten_atomic_store_u32((void*) &oswrapper_image__preload_janky_lock, 0);
}

static void oswrapper_image__load_path_onerror(const char* textureName) {
    (void) textureName;
    /* TODO Unfinished multithreading */
    /* Notify that a callback has been called */
    emscripten_atomic_store_u32((void*) &oswrapper_image__preload_janky_lock, 0);
}

OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_path(const char* path, int* width, int* height, int* channels) {
    volatile uint32_t janky_lock_val;
    int does_file_exist_for_preloading;
    unsigned char* try_get_preloaded_data = oswrapper_image__load_from_path_post_preload(path, width, height, channels);

    if (try_get_preloaded_data != NULL) {
        /* Image is already preloaded */
        return try_get_preloaded_data;
    }

    /* TODO Unfinished multithreading */
    /* Busy acquire the lock */
    do {
        janky_lock_val = emscripten_atomic_cas_u32((void*) &oswrapper_image__preload_janky_lock, 0, 1);
    } while (janky_lock_val);

    does_file_exist_for_preloading = emscripten_run_preload_plugins(path, oswrapper_image__load_path_onload, oswrapper_image__load_path_onerror);

    if (does_file_exist_for_preloading == 0) {
        /* TODO Unfinished multithreading */
        /* Busy wait until the callbacks are called */
        do {
            /* TODO emscripten_run_preload_plugins uses the main thread to load images, which we are probably on.
            Busy waiting will therefore normally deadlock,
            because the image load callbacks will never run without returning to the main thread.
            Loading synchronously can only work if we're using Asyncify,
            because calling emscripten_sleep will allow the main thread to run the preload plugins. */
            emscripten_sleep(100);
            janky_lock_val = emscripten_atomic_load_u32((void*) &oswrapper_image__preload_janky_lock);
        } while (janky_lock_val == 1);

        return oswrapper_image__load_from_path_post_preload(path, width, height, channels);
    }

    emscripten_atomic_store_u32((void*) &oswrapper_image__preload_janky_lock, 0);
    return NULL;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */

#ifdef OSWRAPPER_IMAGE_EXPERIMENTAL
/* TODO This actually copies more than the alternative */
OSWRAPPER_IMAGE_DEF void oswrapper_image_free_nocopy(OSWrapper_image_decoded_data* decoded_data) {
    if (decoded_data != NULL) {
        if (decoded_data->image_data != NULL) {
            oswrapper_image_free(decoded_data->image_data);
        }

        OSWRAPPER_IMAGE_FREE(decoded_data);
    }
}

/* TODO This actually copies more than the alternative */
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_memory_nocopy(unsigned char* image, int length, int* width, int* height, int* channels) {
    unsigned char* image_data = oswrapper_image_load_from_memory(image, length, width, height, channels);

    if (image_data != NULL) {
        OSWrapper_image_decoded_data* decoded_data = (OSWrapper_image_decoded_data*) OSWRAPPER_IMAGE_MALLOC(sizeof(OSWrapper_image_decoded_data));

        if (decoded_data != NULL) {
            decoded_data->internal_data = NULL;
            decoded_data->image_data = image_data;
            return decoded_data;
        }
    }

    return NULL;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
/* TODO This actually copies more than the alternative */
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_path_nocopy(const char* path, int* width, int* height, int* channels) {
    unsigned char* image_data = oswrapper_image_load_from_path(path, width, height, channels);

    if (image_data != NULL) {
        OSWrapper_image_decoded_data* decoded_data = (OSWrapper_image_decoded_data*) OSWRAPPER_IMAGE_MALLOC(sizeof(OSWrapper_image_decoded_data));

        if (decoded_data != NULL) {
            decoded_data->internal_data = NULL;
            decoded_data->image_data = image_data;
            return decoded_data;
        }
    }

    return NULL;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */
#endif
/* End Emscripten preloader implementation */
#else
/* No image loader implementation */
OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_init(void) {
    return OSWRAPPER_IMAGE_RESULT_FAILURE;
}

OSWRAPPER_IMAGE_DEF OSWRAPPER_IMAGE_RESULT_TYPE oswrapper_image_uninit(void) {
    return OSWRAPPER_IMAGE_RESULT_SUCCESS;
}

#ifdef OSWRAPPER_IMAGE_EXPERIMENTAL
OSWRAPPER_IMAGE_DEF void oswrapper_image_free_nocopy(OSWrapper_image_decoded_data* decoded_data) {
    if (decoded_data != NULL) {
        /* ??? */
        OSWRAPPER_IMAGE_FREE(decoded_data);
    }
}

OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_memory_nocopy(unsigned char* image, int length, int* width, int* height, int* channels) {
    return NULL;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
OSWRAPPER_IMAGE_DEF OSWrapper_image_decoded_data* oswrapper_image_load_from_path_nocopy(const char* path, int* width, int* height, int* channels) {
    return NULL;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */
#endif

OSWRAPPER_IMAGE_DEF void oswrapper_image_free(unsigned char* image_data) {
    if (image_data != NULL) {
        /* ??? */
        OSWRAPPER_IMAGE_FREE(image_data);
    }
}

OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_memory(unsigned char* image, int length, int* width, int* height, int* channels) {
    return NULL;
}

#ifndef OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH
OSWRAPPER_IMAGE_DEF unsigned char* oswrapper_image_load_from_path(const char* path, int* width, int* height, int* channels) {
    return NULL;
}
#endif /* OSWRAPPER_IMAGE_NO_LOAD_FROM_PATH */
/* End no image loader implementation */
#endif
#endif /* OSWRAPPER_IMAGE_IMPLEMENTATION */
#endif /* OSWRAPPER_INCLUDE_OSWRAPPER_IMAGE_H */

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
