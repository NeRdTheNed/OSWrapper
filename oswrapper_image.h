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
#else
/* No image loader implementation */
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
