/*
 * MIT License
 *
 * Copyright (c) 2024 hanyazou
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdarg.h>
#include <stdio.h>
#include <gpiow.h>

int gpiow_log_level = GPIOW_LOG_INFO;

void gpiow_log_impl(int level, char *fmt, ...)
{
    va_list arg_ptr;
    char *header;
    static char *level_strings[] = {
        [GPIOW_LOG_ERROR] = "Error",
        [GPIOW_LOG_WARN] = "Warning",
        [GPIOW_LOG_INFO] = "Info",
        [GPIOW_LOG_DEBUG] = "D",
        [GPIOW_LOG_VERBOSE] = "V",
    };
    if (gpiow_log_level < level) {
        return;
    }

    if (0 <= level && level < sizeof(level_strings)/sizeof(*level_strings)) {
        header = level_strings[level];
    } else {
        header = "?";
    }

    printf("%s: ", header);
    va_start(arg_ptr, fmt);
    vprintf(fmt, arg_ptr);
    va_end(arg_ptr);
    printf("\n");
}

void (*gpiow_log_hook)(int level, char *fmt, ...) = gpiow_log_impl;
