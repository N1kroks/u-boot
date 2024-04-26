// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023, Linaro Limited
 */

#if !defined(_SANDBOX_EFI_CAPSULE_H_)
#define _SANDBOX_EFI_CAPSULE_H_

#define SANDBOX_UBOOT_IMAGE_GUID	"fd5db83c-12f3-a45b-9da9-e3007c7ff56e"
#define SANDBOX_UBOOT_ENV_IMAGE_GUID	"935fe837-fac8-4354-8208-737d8852c60d"
#define SANDBOX_FIT_IMAGE_GUID		"ffd97379-0956-fa54-b703-8bfcf5cc097b"
#define SANDBOX_INCORRECT_GUID		"058b7d83-50d5-4c47-a195-60d86ad341c4"

#define UBOOT_FIT_IMAGE			"u-boot_bin_env.itb"

#define CAPSULE_PRIV_KEY		"capsule_priv_key_good.key"
#define CAPSULE_PUB_KEY			"capsule_pub_key_good.crt"
#define CAPSULE_INVAL_KEY		"capsule_priv_key_bad.key"
#define CAPSULE_INVAL_PUB_KEY		"capsule_pub_key_bad.crt"

#endif /* _SANDBOX_EFI_CAPSULE_H_ */
