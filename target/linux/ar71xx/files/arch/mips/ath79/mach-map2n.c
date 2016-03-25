/*
 *  GL-CONNECT iNet board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 alzhao <alzhao@gmail.com>
 *  Copyright (C) 2014 Michel Stempin <michel.stempin@wanadoo.fr>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/etherdevice.h>
#include <linux/ar8216_platform.h>
#include <linux/rle.h>
#include <linux/routerboot.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/gpio.h>

#include <asm/mach-ath79/irq.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-spi.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "routerboot.h"

#define MAP2N_GPIO_LED_WLAN		0
#define MAP2N_GPIO_LED_LAN		13
#define MAP2N_GPIO_BTN_RESET		11

#define MAP2N_KEYS_POLL_INTERVAL	20	/* msecs */
#define MAP2N_KEYS_DEBOUNCE_INTERVAL	(3 * MAP2N_KEYS_POLL_INTERVAL)

#define RB_ROUTERBOOT_OFFSET    0x0000
#define RB_ROUTERBOOT_SIZE      0xe000
#define RB_HARD_CFG_OFFSET      0xe000
#define RB_HARD_CFG_SIZE        0x1000

#define RB_ART_SIZE             0x10000

/*static const char * map2n_part_probes[] = {
	"tp-link", // dont change, this will use tplink parser 
	NULL ,
};*/

static struct mtd_partition map2n_part_probes[] = {
        {
                .name = "RouterBoot",
                .offset = 0,
                .size = 2 * 128 * 1024,
                .mask_flags = MTD_WRITEABLE,
        },
	{
                .name = "kernel",
                .offset = 2 * 128 * 1024,
                .size = 0x600000
        },
	{
                .name = "rootfs",
                .offset = 0x640000,
                .size = MTDPART_SIZ_FULL
        }
};

static struct flash_platform_data map2n_flash_data = {
	.parts 	  = map2n_part_probes,
	.nr_parts = ARRAY_SIZE(map2n_part_probes),
};

static struct gpio_led map2n_leds_gpio[] __initdata = {
	{
		.name = "map2n:red:wlan",
		.gpio = MAP2N_GPIO_LED_WLAN,
		.active_low = 0,
	},
	{
		.name = "map2n:green:lan",
		.gpio = MAP2N_GPIO_LED_LAN,
		.active_low = 0,
		.default_state = 1,
	},
};

/*static struct gpio_keys_button map2n_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = MAP2N_KEYS_DEBOUNCE_INTERVAL,
		.gpio = MAP2N_GPIO_BTN_RESET,
		.active_low = 0,
	}
};*/

static void __init map2n_wlan_init(void)
{
        u8 *hard_cfg = (u8 *) KSEG1ADDR(0x1f000000 + RB_HARD_CFG_OFFSET);
        u16 tag_len;
        u8 *tag;
        char *art_buf;
        u8 wlan_mac[ETH_ALEN];
        int err;

        err = routerboot_find_tag(hard_cfg, RB_HARD_CFG_SIZE, RB_ID_WLAN_DATA,
                                  &tag, &tag_len);
        if (err) {
                pr_err("no calibration data found\n");
                return;
        }

        art_buf = kmalloc(RB_ART_SIZE, GFP_KERNEL);
        if (art_buf == NULL) {
                pr_err("no memory for calibration data\n");
                return;
        }

        err = rle_decode((char *) tag, tag_len, art_buf, RB_ART_SIZE,
                         NULL, NULL);
        if (err) {
                pr_err("unable to decode calibration data\n");
                goto free;
        }

        ath79_init_mac(wlan_mac, ath79_mac_base, 11);
        ath79_register_wmac(art_buf + 0x1000, wlan_mac);

free:
        kfree(art_buf);
}

static void __init map2n_setup(void)
{
	ath79_register_leds_gpio(-1, ARRAY_SIZE(map2n_leds_gpio),
                                 map2n_leds_gpio);

        ath79_register_m25p80(&map2n_flash_data);
        ath79_register_mdio(0, 0x0);

        ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
        ath79_register_eth(0);

        ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);
        ath79_register_eth(1);

        ath79_register_usb();

        map2n_wlan_init();

    gpio_request_one(26,
                         GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_CHANGEABLE,
                         "rb:poe");
}

MIPS_MACHINE(ATH79_MACH_MAP2N, "mAP", "MAP2N v1",
	     map2n_setup);
