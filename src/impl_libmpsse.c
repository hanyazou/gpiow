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
#include <mpsse.h>

#define IMPL_NAME "libmpsse"

#define MIN_ADDR 0x00
#define MAX_ADDR 0x7f
#define VALID_ADDR(addr) ((MIN_ADDR <= (addr) && (addr) <= MAX_ADDR))
#define VALID_HANDLE(hdl) (((hdl) & 0x01) == 0 && (MIN_ADDR * 2 <= (hdl) && (hdl) <= MAX_ADDR * 2))

struct libmpsse_data {
    struct mpsse_context *mpsse;
    int rd_addr;
    int wr_addr;
    int clockspeed;
    int msblsb;
};

static int libmpsse_i2c_open(struct gpw_i2c_bus *bus, int addr, unsigned int flags)
{
    if (bus == NULL || bus->data == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    struct libmpsse_data *priv = (struct libmpsse_data *)bus->data;
    if (priv->mpsse == NULL || !priv->mpsse->open) {
        return GPIOW_RES_INVALID_OBJ;
    }
    if (!VALID_ADDR(addr)) {
        return GPIOW_RES_INVALID_ARGUMENT;
    }

    return (addr << 1);  /* use I2C slave address 0x00 to 0xFE as a handler */
}

static int libmpsse_i2c_read_device(struct gpw_i2c_bus *bus, int handle, unsigned char *data, int size)
{
    if (bus == NULL || bus->data == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    struct libmpsse_data *priv = (struct libmpsse_data *)bus->data;
    if (priv->mpsse == NULL || !priv->mpsse->open || !VALID_HANDLE(handle)) {
        return GPIOW_RES_INVALID_HANDLE;
    }

    int res;
    char rd_addr = (handle | 0x01);
    Start(priv->mpsse);
    if (Write(priv->mpsse, &rd_addr, 1) != MPSSE_OK) {
        res = GPIOW_RES_BACKEND_FAILURE;
        goto wayout;
    }
    if (GetAck(priv->mpsse) != ACK) {
        res = GPIOW_RES_COMMUNICATION_ERROR;
        goto wayout;
    }
    char *read_data = Read(priv->mpsse, size);
    if(read_data == NULL) {
        res = GPIOW_RES_COMMUNICATION_ERROR;
    } else {
        memcpy(data, read_data, size);
        free(read_data);
        res = size;
    }
    SendNacks(priv->mpsse);
    Read(priv->mpsse, 1);
    SendAcks(priv->mpsse);

 wayout:
    Stop(priv->mpsse);

    return res;
}

static int libmpsse_i2c_write_device(struct gpw_i2c_bus *bus, int handle, unsigned char *data, int size)
{
    if (bus == NULL || bus->data == NULL) {
        return GPIOW_RES_INVALID_OBJ;
    }
    struct libmpsse_data *priv = (struct libmpsse_data *)bus->data;
    if (priv->mpsse == NULL || !priv->mpsse->open || !VALID_HANDLE(handle)) {
        return GPIOW_RES_INVALID_HANDLE;
    }

    int res;
    char wr_addr = (handle | 0x00);
    Start(priv->mpsse);
    if (Write(priv->mpsse, &wr_addr, 1) != MPSSE_OK) {
        res = GPIOW_RES_BACKEND_FAILURE;
        goto wayout;
    }
    if (GetAck(priv->mpsse) != ACK) {
        res = GPIOW_RES_COMMUNICATION_ERROR;
        goto wayout;
    }

    if (0 < size) {
        Write(priv->mpsse, (char*)data, size);
        if (GetAck(priv->mpsse) != ACK) {
            res = GPIOW_RES_COMMUNICATION_ERROR;
            goto wayout;
        }
    }
    res = size;

 wayout:
    Stop(priv->mpsse);

    return res;
}

static void libmpsse_i2c_close(struct gpw_i2c_bus *bus, int handle)
{
    /* nothing to do here */
}

static void libmpsse_i2c_release(struct gpw_i2c_bus *bus)
{
    if (bus == NULL || bus->data == NULL) {
        return;
    }
    struct libmpsse_data *priv = (struct libmpsse_data *)bus->data;
    Close(priv->mpsse);
    memset(priv, 0, sizeof(*priv));
    memset(bus, 0, sizeof(*bus));
    free(bus);
}

static struct gpw_i2c_bus libmpsse_i2c_bus_tmpl = {
    .open = libmpsse_i2c_open,
    .read_device = libmpsse_i2c_read_device,
    .write_device = libmpsse_i2c_write_device,
    .close = libmpsse_i2c_close,
    .release = libmpsse_i2c_release,
};

static struct gpw_i2c_bus *libmpsse_i2c_create(char* uri)
{
    int i;
    struct gpw_i2c_bus *bus;

    /* Allocate bus object */
    bus = calloc(1, sizeof(libmpsse_i2c_bus_tmpl) + sizeof(struct libmpsse_data));
    if (bus == NULL) {
        gpiow_log(GPIOW_LOG_ERROR, "%s: memory allocation failed", __func__);
        return NULL;
    }
    memcpy(bus, &libmpsse_i2c_bus_tmpl, sizeof(libmpsse_i2c_bus_tmpl));
    struct libmpsse_data *priv = (struct libmpsse_data *)&bus[1];
    bus->data = priv;

    priv->clockspeed = FOUR_HUNDRED_KHZ;
    priv->msblsb = MSB;

    /* Try to connect pigpiod with the library default parameters if URI is not specified */
    if (uri == NULL) {
        priv->mpsse = MPSSE(I2C, priv->clockspeed, priv->msblsb);
        if (priv->mpsse != NULL && priv->mpsse->open) {
            return bus;
        }
        Close(priv->mpsse);
        priv->mpsse = NULL;
    }

    char vid[5] = {0};
    char pid[5] = {0};
    char *ptr = uri;

    /* Check the schema in URI if URI is not null */
    if (gpiow_uri_string(&ptr, NULL, 0, IMPL_NAME ":", GPIOW_URI_MATCH_EXACT) <= 0) {
        goto not_match;
    }

    if (0 < gpiow_uri_string(&ptr, NULL, 0, "//", GPIOW_URI_MATCH_EXACT)) {
        gpiow_uri_string(&ptr, vid, sizeof(vid), ":/", GPIOW_URI_UNMATCH_CHARS);
        if (0 < gpiow_uri_string(&ptr, NULL, 0, ":", GPIOW_URI_MATCH_EXACT)) {
            gpiow_uri_string(&ptr, pid, sizeof(pid), "/", GPIOW_URI_UNMATCH_CHARS);
        }
    }

    /*
     * TODO: parse URI for clock speed and so on and handle VID:PID
     */

    gpiow_uri_string(&ptr, NULL, 0, "/", GPIOW_URI_MATCH_EXACT);
    if (*ptr != '\0') {
        goto malformed_uri;
    }

    gpiow_log(GPIOW_LOG_INFO, "%s: %s%s%s clockspeed=%d, %s", __func__,
              vid, (*vid || *pid) ? ":" : "", pid,
              priv->clockspeed, priv->msblsb == MSB ? "MSB" : "LSB");
    priv->mpsse = MPSSE(I2C, priv->clockspeed, priv->msblsb);
    if (priv->mpsse == NULL || !priv->mpsse->open) {
        Close(priv->mpsse);
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

static struct gpw_i2c_impl_entry libmpsse_entry = {
    .name = IMPL_NAME,
    .create = libmpsse_i2c_create,
};

void gpiow_libmpsse_initialize(void)
{
    gpw_i2c_bus_register(&libmpsse_entry);
}
