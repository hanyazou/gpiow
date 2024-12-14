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

#include <stdio.h>
#include <string.h>
#include <gpiow/uri.h>

int gpiow_uri_schema(char **uri, char *buf, int len)
{
    if (uri == NULL || *uri == NULL) {
        return GPIOW_RES_INVALID_URI;
    }

    char *p = *uri;
    while (*p != '\0' && *p !=  ':') {
        if (1 < len) {
            *buf++ = *p;
            len--;
        }
        p++;
    }
    if (*p == ':') {
        p++;
    }
    if (0 < len) {
        *buf = '\0';
    }
    *uri = p;

    return GPIOW_RES_OK;
}

int gpiow_uri_addr(char **uri, char *buf, int len)
{
    if (uri == NULL || *uri == NULL) {
        return GPIOW_RES_INVALID_URI;
    }

    char *p = *uri;
    if (strncmp(p, "//", 2) == 0) {
        p += 2;
        while (*p != '\0' && *p !=  ':' && *p !=  '/') {
            if (1 < len) {
                *buf++ = *p;
                len--;
            }
            p++;
        }
    }
    if (0 < len) {
        *buf = '\0';
    }
    *uri = p;

    return GPIOW_RES_OK;
}

int gpiow_uri_port(char **uri, char *buf, int len)
{
    if (uri == NULL || *uri == NULL) {
        return GPIOW_RES_INVALID_URI;
    }

    char *p = *uri;
    if (*p == ':') {
        p++;
        while (*p != '\0' && *p !=  '/') {
            if (1 < len) {
                *buf++ = *p;
                len--;
            }
            p++;
        }
    }
    if (*p == '/') {
        p++;
    }
    if (0 < len) {
        *buf = '\0';
    }
    *uri = p;

    return GPIOW_RES_OK;
}
