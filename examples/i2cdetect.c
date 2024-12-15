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
#include <stdlib.h>
#include <unistd.h>

#include <gpiow/gpiow.h>

int main(int argc, char *argv[])
{
    struct gpw_i2c_bus *i2c_bus;
    int handle;

    gpiow_initialize();
    i2c_bus = gpw_i2c_bus_create((1 < argc) ? argv[1] : NULL);
    if (i2c_bus == NULL) {
        exit(1);
    }

    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (int addr = 0x00; addr <= 0x7f; addr++) {
        if((addr & 0x0f) == 0x00) {
            printf("%02x: ", addr);
        }
        if (0x08 <= addr && addr <= 0x77) {
            handle = gpw_i2c_open(i2c_bus, addr, 0);
            if (handle < 0) {
                printf("   ");
            } else {
                uint8_t tmp;
                if (gpw_i2c_write_device(i2c_bus, handle, &tmp, 0) < 0) {
                    printf("-- ");
                } else {
                    printf("%02x ", addr);
                }
            }
            gpw_i2c_close(i2c_bus, handle);
        } else {
            printf("   ");
        }
        if((addr & 0x0f) == 0x0f) {
            printf("\n");
        }
    }

    gpw_i2c_bus_release(i2c_bus);

    exit(0);
}
