// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#include <env_internal.h>
#include <spl.h>
#include <asm/arch/hardware.h>

#include "../am6_som_detection.h"

#if IS_ENABLED(CONFIG_ENV_IS_IN_FAT) || IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
int mmc_get_env_dev(void)
{
	u32 boot_device = get_boot_device();

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	};

	return CONFIG_SYS_MMC_ENV_DEV;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 boot_device = get_boot_device();

	if (prio)
		return ENVL_UNKNOWN;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_FAT))
			return ENVL_FAT;
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;
	case BOOT_DEVICE_SPI:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
	default:
		return ENVL_NOWHERE;
	};
}

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
	u32 boot_device = get_boot_device();

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		env_set_ulong("mmcdev", 0);
		env_set("boot", "mmc");
		break;
	case BOOT_DEVICE_MMC2:
		env_set_ulong("mmcdev", 1);
		env_set("boot", "mmc");
		break;
	case BOOT_DEVICE_SPI:
		env_set("boot", "spi");
		break;
	case BOOT_DEVICE_ETHERNET:
		env_set("boot", "net");
		break;
	};

	if (IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION_BLOCKS)) {
		struct phytec_api3_element *block_element;
		struct phytec_eeprom_data data;
		int ret;

		ret = phytec_eeprom_data_setup(&data, 0, EEPROM_ADDR);
		if (ret || !data.valid)
			return 0;

		PHYTEC_API3_FOREACH_BLOCK(block_element, &data) {
			switch (block_element->block_type) {
			case PHYTEC_API3_BLOCK_MAC:
				phytec_blocks_add_mac_to_env(block_element);
				break;
			default:
				debug("%s: Unknown block type %i\n", __func__,
				      block_element->block_type);
			}
		}
	}

	return 0;
}
#endif
