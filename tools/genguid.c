// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2024 Linaro Ltd.
 *   Author: Caleb Connolly
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>

#include <uuid.h>

static struct option options[] = {
	{"dtb", required_argument, NULL, 'd'},
	{"compat", required_argument, NULL, 'c'},
	{"help", no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{"json", no_argument, NULL, 'j'},
	{NULL, 0, NULL, 0},
};

static void usage(const char *progname)
{
	fprintf(stderr, "Usage: %s GUID [-v] -c COMPAT NAME...\n", progname);
	fprintf(stderr,
		"Generate a v5 GUID for one of more U-Boot fw_images the same way U-Boot does at runtime.\n");
	fprintf(stderr,
		"\nOptions:\n"
		"  GUID                     namespace/salt GUID in 8-4-4-4-12 format\n"
		"  -h, --help               display this help and exit\n"
		"  -c, --compat=COMPAT      first compatible property in the board devicetree\n"
		"  -v, --verbose            print debug messages\n"
		"  -j, --json               output in JSON format\n"
		"  NAME...                  one or more names of fw_images to generate GUIDs for\n"
	);
	fprintf(stderr, "\nExample:\n");
	fprintf(stderr, "  %s 2a5aa852-b856-4d97-baa9-5c5f4421551f \\\n"
			"\t-c \"qcom,qrb4210-rb2\" \\\n"
			"\tQUALCOMM-UBOOT\n", progname);
}

static size_t u16_strsize(const uint16_t *in)
{
	size_t i = 0, count = UINT16_MAX;

	while (count-- && in[i])
		i++;

	return (i + 1) * sizeof(uint16_t);
}

int main(int argc, char **argv)
{
	struct uuid namespace;
	char *namespace_str;
	char uuid_str[37];
	char **image_uuids;
	char *compatible = NULL;
	uint16_t **images_u16;
	char **images;
	int c, n_images;
	bool debug = false, json = false;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	namespace_str = argv[1];

	/* The first arg is the GUID so skip it */
	while ((c = getopt_long(argc, argv, "c:hvj", options, NULL)) != -1) {
		switch (c) {
		case 'c':
			compatible = strdup(optarg);
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		case 'v':
			debug = true;
			break;
		case 'j':
			json = true;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (!compatible) {
		fprintf(stderr, "ERROR: Please specify the compatible property.\n\n");
		usage(argv[0]);
		return 1;
	}

	if (uuid_str_to_bin(namespace_str, (unsigned char *)&namespace, UUID_STR_FORMAT_GUID)) {
		fprintf(stderr, "ERROR: Check that your UUID is formatted correctly.\n");
		exit(EXIT_FAILURE);
	}

	/* This is probably not the best way to convert a string to a "u16" string */
	n_images = argc - optind - 1;
	images = argv + optind + 1;
	images_u16 = calloc(n_images, sizeof(char *));
	for (int i = 0; i < n_images; i++) {
		images_u16[i] = calloc(1, strlen(images[i]) * 2 + 2);
		for (int j = 0; j < strlen(images[i]); j++)
			images_u16[i][j] = (uint16_t)images[i][j];
	}

	if (debug) {
		fprintf(stderr, "GUID:         ");
		uuid_bin_to_str((uint8_t *)&namespace, uuid_str, UUID_STR_FORMAT_GUID);
		fprintf(stderr, "%s\n", uuid_str);
		fprintf(stderr, "Compatible:  \"%s\"\n", compatible);
		fprintf(stderr, "Images:      ");
		for (int i = 0; i < n_images; i++)
			fprintf(stderr, "\"%s\"%s", argv[optind + i + 1],
				i == n_images - 1 ? "\n" : ", ");
	}

	image_uuids = calloc(n_images, sizeof(char *));
	for (int i = 0; i < n_images; i++) {
		struct uuid image_type_id;

		gen_uuid_v5(&namespace, &image_type_id,
			    compatible, strlen(compatible),
			    images_u16[i], u16_strsize(images_u16[i]) - sizeof(uint16_t),
			    NULL);

		uuid_bin_to_str((uint8_t *)&image_type_id, uuid_str, UUID_STR_FORMAT_GUID);
		image_uuids[i] = strdup(uuid_str);
	}

	if (json) {
		printf("[\n");
		for (int i = 0; i < n_images; i++)
			printf("\t{\"name\": \"%s\", \"uuid\": \"%s\"}%s\n", images[i], image_uuids[i],
			       i == n_images - 1 ? "" : ",");
		printf("]\n");
	} else {
		for (int i = 0; i < n_images; i++)
			printf("%-24s| %s\n", images[i], image_uuids[i]);
	}

	return 0;
}

