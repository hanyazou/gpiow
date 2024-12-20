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

#ifndef __GPIOW_MULTI_IMPL_H__
#define __GPIOW_MULTI_IMPL_H__

struct gpw_i2c_impl_entry {
    char *name;
    struct gpw_i2c_impl_entry *next;
    struct gpw_i2c_bus* (*create)(char* uri);
};

struct gpw_i2c_bus {
    void *data;
    int (*open)(struct gpw_i2c_bus*, int addr, unsigned int flags);
    int (*read_device)(struct gpw_i2c_bus*, int handle, unsigned char *data, int size);
    int (*write_device)(struct gpw_i2c_bus*, int handle, unsigned char *data, int size);
    void (*close)(struct gpw_i2c_bus*, int handle);
    void (*release)(struct gpw_i2c_bus*);
};

void gpw_i2c_bus_register(struct gpw_i2c_impl_entry *entry);

#endif  /* __GPIOW_MULTI_IMPL_H__ */
