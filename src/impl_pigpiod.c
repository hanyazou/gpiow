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

#include <stdlib.h>
#include <string.h>
#include <gpiow/gpiow.h>
#include <gpiow/multi_impl.h>
#include <gpiow/uri.h>
#include <pigpiod_if2.h>

#define IMPL_NAME "pigpiod"

struct pigpiod_i2c_data {
    int pi;
    int busnum;
};

static int pigpiod_i2c_open(struct gpw_i2c_bus *bus, int addr, unsigned int flags)
{
    if (bus == NULL || bus->data == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    struct pigpiod_i2c_data *priv = (struct pigpiod_i2c_data *)bus->data;
    return i2c_open(priv->pi, priv->busnum, addr, 0);
}

static int pigpiod_i2c_read_device(struct gpw_i2c_bus *bus, int handle, unsigned char *data, int size)
{
    if (bus == NULL || bus->data == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    struct pigpiod_i2c_data *priv = (struct pigpiod_i2c_data *)bus->data;
    return i2c_read_device(priv->pi, handle, (char *)data, size);
}

static int pigpiod_i2c_write_device(struct gpw_i2c_bus *bus, int handle, unsigned char *data, int size)
{
    if (bus == NULL || bus->data == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    struct pigpiod_i2c_data *priv = (struct pigpiod_i2c_data *)bus->data;
    return i2c_write_device(priv->pi, handle, (char *)data, size);
}

static void pigpiod_i2c_close(struct gpw_i2c_bus *bus, int handle)
{
    if (bus == NULL || bus->data == NULL) {
        return;
    }
    struct pigpiod_i2c_data *priv = (struct pigpiod_i2c_data *)bus->data;
    i2c_close(priv->pi, handle);
}

static void pigpiod_i2c_release(struct gpw_i2c_bus *bus)
{
    if (bus == NULL || bus->data == NULL) {
        return;
    }
    struct pigpiod_i2c_data *priv = (struct pigpiod_i2c_data *)bus->data;
    pigpio_stop(priv->pi);
    memset(priv, 0, sizeof(*priv));
    memset(bus, 0, sizeof(*bus));
    free(bus);
}

static struct gpw_i2c_bus pigpiod_i2c_bus_tmpl = {
    .open = pigpiod_i2c_open,
    .read_device = pigpiod_i2c_read_device,
    .write_device = pigpiod_i2c_write_device,
    .close = pigpiod_i2c_close,
    .release = pigpiod_i2c_release,
};

static struct gpw_i2c_bus *pigpiod_i2c_create(char* uri)
{
    int i;
    struct gpw_i2c_bus *bus;

    /* Allocate bus object */
    bus = calloc(1, sizeof(pigpiod_i2c_bus_tmpl) + sizeof(struct pigpiod_i2c_data));
    if (bus == NULL) {
        gpiow_log(GPIOW_LOG_ERROR, "%s: memory allocation failed", __func__);
        return NULL;
    }
    memcpy(bus, &pigpiod_i2c_bus_tmpl, sizeof(pigpiod_i2c_bus_tmpl));
    struct pigpiod_i2c_data *priv = (struct pigpiod_i2c_data *)&bus[1];
    bus->data = priv;

    /* Try to connect pigpiod with the library default parameters if URI is not specified */
    if (uri == NULL) {
        priv->pi = pigpio_start(NULL, NULL);
        if (priv->pi < 0) {
            goto not_match;
        }
        priv->busnum = 1;
        return bus;
    }

    char addr[128] = {0};
    char port[8] = {0};
    char *tail;
    char *ptr = uri;

    /* Check the schema in URI if URI is not null */
    if (gpiow_uri_string(&ptr, NULL, 0, IMPL_NAME ":", GPIOW_URI_MATCH_EXACT) <= 0) {
        goto not_match;
    }

    if (0 < gpiow_uri_string(&ptr, NULL, 0, "//", GPIOW_URI_MATCH_EXACT)) {
        gpiow_uri_string(&ptr, addr, sizeof(addr), ":/", GPIOW_URI_UNMATCH_CHARS);
        if (0 < gpiow_uri_string(&ptr, NULL, 0, ":", GPIOW_URI_MATCH_EXACT)) {
            gpiow_uri_string(&ptr, port, sizeof(port), "/", GPIOW_URI_UNMATCH_CHARS);
        }
    }

    if (addr[0] || port[0]) {
        gpiow_log(GPIOW_LOG_INFO, "%s: pigpio_start(%s, %s)", __func__, addr, port);

        (void)strtol(port, &tail, 10);
        if (*tail != '\0') {
            goto malformed_uri;
        }
    }
    if (*uri != '\0') {
        gpiow_uri_string(&ptr, NULL, 0, "/", GPIOW_URI_MATCH_EXACT);
        gpiow_log(GPIOW_LOG_INFO, "%s: bus number is \"%s\"", __func__, ptr);
        priv->busnum = strtol(ptr, &tail, 10);
        if (*tail != '\0') {
            goto malformed_uri;
        }
    } else {
        priv->busnum = 1;  /* Raspberry Pi's external I2C pins in the pin header */
    }

    gpiow_log(GPIOW_LOG_INFO, "%s: addr=%s, port=%s, bus=%d", __func__, addr, port, priv->busnum);
    priv->pi = pigpio_start(*addr ? addr : NULL, *port ? port : NULL);
    if (priv->pi < 0) {
        gpiow_log(GPIOW_LOG_ERROR, "%s: pigpio_start(\"%s\", \"%s\") failed", __func__,
                  addr, port, pigpio_error(priv->pi));
        goto error;
    }

    return bus;

 malformed_uri:
    gpiow_log(GPIOW_LOG_ERROR, "%s: malformed URI, \"%s\"", __func__, uri);

 error:
 not_match:
    memset(priv, 0, sizeof(*priv));
    memset(bus, 0, sizeof(*bus));
    free(bus);

    return NULL;
}

static struct gpw_i2c_impl_entry pigpiod_entry = {
    .name = IMPL_NAME,
    .create = pigpiod_i2c_create,
};

void gpiow_pigpiod_initialize(void)
{
    gpw_i2c_bus_register(&pigpiod_entry);
}
