/**
 * @file StbImpl.cpp
 * @brief Single compilation unit for stb_image implementation.
 *
 * This file defines the STB_IMAGE_IMPLEMENTATION and STB_IMAGE_WRITE_IMPLEMENTATION
 * macros before including the stb headers. This instantiates the implementation code
 * for image loading and writing, following the stb_image header-only library pattern.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image.h>
#include <stb_image_write.h>

// The compiler will write here the implementation of the stb functions
