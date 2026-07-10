#ifndef LAME_CONFIG_H
#define LAME_CONFIG_H

/*
 * Maintained configuration for the direct Makefile build.
 *
 * This is not an autoconf output file. Keep only settings that are active in
 * the current source tree, and put platform-specific frontend choices in the
 * Makefile.
 */

#define STDC_HEADERS 1
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STDINT_H 1

typedef float ieee754_float32_t;
typedef double ieee754_float64_t;

#endif /* LAME_CONFIG_H */
