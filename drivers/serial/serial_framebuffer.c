// SPDX-License-Identifier: GPL-2.0+
/*
 * Framebuffer UART driver
 *
 * (C) Copyright 2024 Nikroks <nikroksm@mail.ru>
 *
 * Based on uniLoader framebuffer driver
 */

#include <serial.h>
#include <misc.h>
#include <dm.h>
#include <errno.h>
#include "font.h"

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 2400

DECLARE_GLOBAL_DATA_PTR;

struct framebuffer_serial_data {
    phys_addr_t base;
};

int curr_x = 0;
int curr_y = 0;

static int framebuffer_serial_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int framebuffer_serial_getc(struct udevice *dev)
{
	return -EAGAIN;
}

static int framebuffer_serial_pending(struct udevice *dev, bool input)
{
	return 0;
}

static void clear_fb(struct udevice *dev) {
    struct framebuffer_serial_data *priv = dev_get_priv(dev);
    char *base = (char*)priv->base;
    for(int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT * 4); i++) {
        base[i] = 0;
    }
}

static void fb_putc(struct udevice *dev, const char c) {
    struct framebuffer_serial_data *priv = dev_get_priv(dev);

    if(c < 32) {
        if(c == '\n') {
            curr_y++;
            curr_x = 0;
        } else if (c == '\r') {
            curr_x = 0;
        }
        return;
    }

    int ix = font_index(c);
    unsigned char *img = letters[ix];

    for(int y = 0; y < FONTH; y++) {
        unsigned char b = img[y];
        for(int x =0; x < FONTW; x++) {
            if (((b << x) & 0b10000000) > 0) {
                char *base = (char*)priv->base;
                long int loc = ((curr_x * FONTW + x) * 4) + ((curr_y * FONTH + y) * SCREEN_WIDTH * 4);
                base[loc] = 255;
                base[loc + 1] = 255;
                base[loc + 2] = 255;
                base[loc + 3] = 255;
            }
        }
    }
    if(curr_x > SCREEN_WIDTH / FONTW) {
        curr_y++;
        curr_x = 0;
    } else if (curr_y > SCREEN_HEIGHT / FONTH) {
        curr_x = 0;
        curr_y = 0;
        clear_fb(dev);
    } else {
        curr_x++;
    }
	return;
}

static int framebuffer_serial_putc(struct udevice *dev, const char ch)
{
    fb_putc(dev, ch);
    return 0;
}

static int framebuffer_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct framebuffer_serial_data *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct udevice_id framebuffer_serial_ids[] = {
	{ .compatible = "framebuffer-serial" },
	{ }
};


const struct dm_serial_ops framebuffer_serial_ops = {
	.putc = framebuffer_serial_putc,
	.pending = framebuffer_serial_pending,
	.getc = framebuffer_serial_getc,
	.setbrg = framebuffer_serial_setbrg,
};

U_BOOT_DRIVER(serial_framebuffer) = {
	.name	= "serial_framebuffer",
	.id	= UCLASS_SERIAL,
	.of_match = framebuffer_serial_ids,
	.of_to_plat = framebuffer_serial_ofdata_to_platdata,
	.priv_auto = sizeof(struct framebuffer_serial_data),
	.ops = &framebuffer_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_FRAMEBUFFER

static struct framebuffer_serial_data init_serial_data = {
	.base = CONFIG_VAL(DEBUG_UART_BASE)
};

/* Serial dumb device, to reuse driver code */
static struct udevice init_dev = {
	.priv_ = &init_serial_data,
};

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
    clear_fb(&init_dev);
}

static inline void _debug_uart_putc(int ch)
{
    fb_putc(&init_dev, ch);
}

DEBUG_UART_FUNCS

#endif