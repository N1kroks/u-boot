// SPDX-License-Identifier: GPL-2.0+
/*
 * Functional tests for UCLASS_FFA  class
 *
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <charset.h>
#include <uuid.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

/* test UUID */
#define TEST_SVC_UUID	"ed32d533-4209-99e6-2d72-cdd998a79cc0"
/* Sandbox namespace UUID */
#define SANDBOX_NAMESPACE_UUID	"09d7cf52-0720-4710-91d1-08469b7fe9c8"
/* Qcom namespace UUID */
#define QCOM_NAMESPACE_UUID	"2a5aa852-b856-4d97-baa9-5c5f4421551f"

#define UUID_SIZE 16

/* The UUID binary data (little-endian format) */
static const u8 ref_uuid_bin[UUID_SIZE] = {
	0x33, 0xd5, 0x32, 0xed,
	0x09, 0x42, 0xe6, 0x99,
	0x72, 0x2d, 0xc0, 0x9c,
	0xa7, 0x98, 0xd9, 0xcd
};

static int lib_test_uuid_to_le(struct unit_test_state *uts)
{
	const char *uuid_str = TEST_SVC_UUID;
	u8 ret_uuid_bin[UUID_SIZE] = {0};

	ut_assertok(uuid_str_to_le_bin(uuid_str, ret_uuid_bin));
	ut_asserteq_mem(ref_uuid_bin, ret_uuid_bin, UUID_SIZE);

	return 0;
}

LIB_TEST(lib_test_uuid_to_le, 0);

#if CONFIG_IS_ENABLED(EFI_CAPSULE_DYNAMIC_UUIDS)
struct dynamic_uuid_test_data {
	const char *namespace;
	const char *compatible;
	const u16 *images[4];
	const char *expected_uuids[4];
};

static int lib_test_dynamic_uuid_case(struct unit_test_state *uts,
				      const struct dynamic_uuid_test_data *data)
{
	struct uuid namespace;
	int j;

	ut_assertok(uuid_str_to_bin(data->namespace, (unsigned char *)&namespace,
				    UUID_STR_FORMAT_GUID));

	for (j = 0; data->images[j]; j++) {
		const char *expected_uuid = data->expected_uuids[j];
		const u16 *image = data->images[j];
		struct uuid uuid;
		char uuid_str[37];

		gen_uuid_v5(&namespace, &uuid,
			    data->compatible, strlen(data->compatible),
			    image, u16_strsize(image) - sizeof(uint16_t),
			    NULL);
		uuid_bin_to_str((unsigned char *)&uuid, uuid_str, UUID_STR_FORMAT_GUID);

		ut_asserteq_str(expected_uuid, uuid_str);
	}

	return 0;
}

static int lib_test_dynamic_uuid(struct unit_test_state *uts)
{
	int ret, i;
	const struct dynamic_uuid_test_data test_data[] = {
		{
			.compatible = "sandbox",
			.namespace = SANDBOX_NAMESPACE_UUID,
			.images = {
				u"SANDBOX-UBOOT",
				u"SANDBOX-UBOOT-ENV",
				u"SANDBOX-FIT",
				NULL,
			},
			.expected_uuids = {
				"fd5db83c-12f3-a45b-9da9-e3007c7ff56e",
				"935fe837-fac8-4354-8208-737d8852c60d",
				"ffd97379-0956-fa54-b703-8bfcf5cc097b",
				NULL,
			}
		},
		{
			.compatible = "qcom,qrb4210-rb2",
			.namespace = QCOM_NAMESPACE_UUID,
			.images = {
				u"QUALCOMM-UBOOT",
				NULL,
			},
			.expected_uuids = {
				"8ee418dc-7e00-e156-92a7-274fbbc05ba8",
				NULL,
			}
		},
	};

	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		ret = lib_test_dynamic_uuid_case(uts, &test_data[i]);
		if (ret)
			return ret;
	}

	return 0;
}

LIB_TEST(lib_test_dynamic_uuid, 0);

#endif /* CONFIG_IS_ENABLED(EFI_CAPSULE_DYNAMIC_UUIDS) */
