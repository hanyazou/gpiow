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
#include <gpiow/gpiow.h>
#include <gpiow/multi_impl.h>

static struct gpw_i2c_impl_entry *impl_list = NULL;

extern void gpiow_pigpiod_initialize(void);

void gpiow_initialize(void)
{
    gpiow_pigpiod_initialize();
}

void gpw_i2c_bus_register(struct gpw_i2c_impl_entry *entry)
{
    struct gpw_i2c_impl_entry **p = &impl_list;

    while (*p) {
        if (*p == entry || ((*p)->name && entry->name && strcmp((*p)->name, entry->name) == 0)) {
            gpiow_log(GPIOW_LOG_ERROR, "%s: %s is alredy registered", __func__, entry->name);
            return;
        }
    }
    gpiow_log(GPIOW_LOG_DEBUG, "%s: %s is registered", __func__, entry->name);
    entry->next = impl_list;
    impl_list = entry;
}

struct gpw_i2c_bus *gpw_i2c_bus_create(char* uri)
{
    struct gpw_i2c_impl_entry *impl = impl_list;
    struct gpw_i2c_bus *bus;

    while (impl) {
        if (impl->create && (bus = (*impl->create)(uri)) != NULL) {
            return bus;
        }
        impl = impl->next;
    }

    gpiow_log(GPIOW_LOG_WARN, "%s: can't create instance for %s", __func__, uri);

    return NULL;
}

int gpw_i2c_open(struct gpw_i2c_bus *bus, int addr, unsigned int flags)
{
    if (bus == NULL || bus->open == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    return (*bus->open)(bus, addr, flags);
}

int gpw_i2c_read_device(struct gpw_i2c_bus *bus, int handle, unsigned char *data, int size)
{
    if (bus == NULL || bus->read_device == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    return (*bus->read_device)(bus, handle, data, size);
}

int gpw_i2c_write_device(struct gpw_i2c_bus *bus, int handle, unsigned char *data, int size)
{
    if (bus == NULL || bus->write_device == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    return (*bus->write_device)(bus, handle, data, size);
}

void gpw_i2c_close(struct gpw_i2c_bus *bus, int handle)
{
    if (bus == NULL || bus->close == NULL) {
        return;
    }
    return (*bus->close)(bus, handle);
}

void gpw_i2c_bus_release(struct gpw_i2c_bus *bus)
{
    if (bus == NULL || bus->close == NULL) {
        return;
    }
    return (*bus->release)(bus);
}
