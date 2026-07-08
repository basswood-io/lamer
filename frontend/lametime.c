/*
 *	Lame time routines source file
 *
 *	Copyright (c) 2000 Mark Taylor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined(_WIN32)
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <fcntl.h>
# include <io.h>
# include <windows.h>
#else
# include <sys/time.h>
#endif

#include <stdio.h>
#include <time.h>

#include "lametime.h"

double
GetCPUTime(void)
{
    return clock() / (double) CLOCKS_PER_SEC;
}

double
GetRealTime(void)
{
#if defined(_WIN32)
    FILETIME ft;
    ULARGE_INTEGER t;

    GetSystemTimeAsFileTime(&ft);
    t.LowPart = ft.dwLowDateTime;
    t.HighPart = ft.dwHighDateTime;
    return (double) t.QuadPart * 1.e-7;
#else
    struct timeval t;
    if (gettimeofday(&t, NULL) != 0) {
        return (double) time(NULL);
    }
    return (double) t.tv_sec + 1.e-6 * (double) t.tv_usec;
#endif
}

int
lame_set_stream_binary_mode(FILE * const fp)
{
#if defined(_WIN32)
    _setmode(_fileno(fp), _O_BINARY);
#else
    (void) fp;
#endif
    return 0;
}
