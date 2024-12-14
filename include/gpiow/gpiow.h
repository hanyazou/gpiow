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

#ifndef GPIOW_H__
#define GPIOW_H__

#define GPIOW_MAJOR_VER 0
#define GPIOW_MINOR_VER 5

enum gpiow_result {
    GPIOW_RES_OK = 0,
    GPIOW_RES_INVALID_OBJ = -1,
    GPIOW_RES_INVALID_HANDLE = -2,
    GPIOW_RES_INVALID_URI = -3,
};
enum gpiow_log_level {
    GPIOW_LOG_ERROR,
    GPIOW_LOG_WARN,
    GPIOW_LOG_INFO,
    GPIOW_LOG_DEBUG,
    GPIOW_LOG_VERBOSE,
};

extern void gpiow_initialize(void);
extern char *gpiow_error(int errno);

#ifdef GPIOW_LOG_HOOK
extern void (*gpiow_log_hook)(int level, char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
#define gpiow_log(level, fmt ...) do { (*gpiow_log_hook)(level, fmt); } while (0)
#else
extern void gpiow_log(int level, char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
extern int gpiow_log_level;
#endif

struct gpw_i2c_bus;
extern struct gpw_i2c_bus *gpw_i2c_bus_create(char* uri);
extern int gpw_i2c_open(struct gpw_i2c_bus *, int addr, unsigned int flags);
extern int gpw_i2c_read_device(struct gpw_i2c_bus *, int handle, unsigned char *data, int size);
extern int gpw_i2c_write_device(struct gpw_i2c_bus *, int handle, unsigned char *data, int size);
extern void gpw_i2c_close(struct gpw_i2c_bus *, int handle);
extern void gpw_i2c_bus_release(struct gpw_i2c_bus *);

#endif  /* GPIOW_H__ */
