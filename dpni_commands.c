/*
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 * Author: German Rivera <German.Rivera@freescale.com>
 *	   Lijun Pan <Lijun.Pan@freescale.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation  and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of any
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include "restool.h"
#include "utils.h"
#include "fsl_dpni.h"

#define ALL_DPNI_OPTS (					\
	DPNI_OPT_ALLOW_DIST_KEY_PER_TC |		\
	DPNI_OPT_TX_CONF_DISABLED |			\
	DPNI_OPT_PRIVATE_TX_CONF_ERROR_DISABLED |	\
	DPNI_OPT_DIST_HASH |				\
	DPNI_OPT_DIST_FS |				\
	DPNI_OPT_UNICAST_FILTER	|			\
	DPNI_OPT_MULTICAST_FILTER |			\
	DPNI_OPT_VLAN_FILTER |				\
	DPNI_OPT_IPR |					\
	DPNI_OPT_IPF |					\
	DPNI_OPT_VLAN_MANIPULATION |			\
	DPNI_OPT_QOS_MASK_SUPPORT |			\
	DPNI_OPT_FS_MASK_SUPPORT)

enum mc_cmd_status mc_status;

/**
 * dpni info command options
 */
enum dpni_info_options {
	INFO_OPT_HELP = 0,
	INFO_OPT_VERBOSE,
};

static struct option dpni_info_options[] = {
	[INFO_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	[INFO_OPT_VERBOSE] = {
		.name = "verbose",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dpni_info_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

/**
 * dpni create command options
 */
enum dpni_create_options {
	CREATE_OPT_HELP = 0,
	CREATE_OPT_OPTIONS,
	CREATE_OPT_MAC_ADDR,
	CREATE_OPT_MAX_SENDERS,
	CREATE_OPT_MAX_TCS,
	CREATE_OPT_MAX_DIST_PER_TC,
	CREATE_OPT_MAX_UNICAST_FILTERS,
	CREATE_OPT_MAX_MULTICAST_FILTERS,
	CREATE_OPT_MAX_VLAN_FILTERS,
	CREATE_OPT_MAX_QOS_ENTRIES,
	CREATE_OPT_MAX_QOS_KEY_SIZE,
	CREATE_OPT_MAX_DIST_KEY_SIZE,
};

static struct option dpni_create_options[] = {
	[CREATE_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_OPTIONS] = {
		.name = "options",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAC_ADDR] = {
		.name = "mac-addr",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_SENDERS] = {
		.name = "max-senders",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_TCS] = {
		.name = "max-tcs",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_DIST_PER_TC] = {
		.name = "max-dist-per-tc",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_UNICAST_FILTERS] = {
		.name = "max-unicast-filters",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_MULTICAST_FILTERS] = {
		.name = "max-multicast-filters",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_VLAN_FILTERS] = {
		.name = "max-vlan-filters",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_QOS_ENTRIES] = {
		.name = "max-qos-entries",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_QOS_KEY_SIZE] = {
		.name = "max-qos-key-size",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_MAX_DIST_KEY_SIZE] = {
		.name = "max-dist-key-size",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dpni_create_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

/**
 * dpni destroy command options
 */
enum dpni_destroy_options {
	DESTROY_OPT_HELP = 0,
};

static struct option dpni_destroy_options[] = {
	[DESTROY_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dpni_destroy_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

static const struct flib_ops dpni_ops = {
	.obj_open = dpni_open,
	.obj_close = dpni_close,
	.obj_get_irq_mask = dpni_get_irq_mask,
	.obj_get_irq_status = dpni_get_irq_status,
};

static int cmd_dpni_help(void)
{
	static const char help_msg[] =
		"\n"
		"restool dpni <command> [--help] [ARGS...]\n"
		"Where <command> can be:\n"
		"   info - displays detailed information about a DPNI object.\n"
		"   create - creates a new child DPNI under the root DPRC.\n"
		"   destroy - destroys a child DPNI under the root DPRC.\n"
		"\n"
		"For command-specific help, use the --help option of each command.\n"
		"\n";

	printf(help_msg);
	return 0;
}

static void print_dpni_options(uint32_t options)
{
	if (options == 0 || (options & ~ALL_DPNI_OPTS) != 0) {
		printf("\tUnrecognized options found...\n");
		return;
	}

	if (options & DPNI_OPT_ALLOW_DIST_KEY_PER_TC)
		printf("\tDPNI_OPT_ALLOW_DIST_KEY_PER_TC\n");

	if (options & DPNI_OPT_TX_CONF_DISABLED)
		printf("\tDPNI_OPT_TX_CONF_DISABLED\n");

	if (options & DPNI_OPT_PRIVATE_TX_CONF_ERROR_DISABLED)
		printf("\tDPNI_OPT_PRIVATE_TX_CONF_ERROR_DISABLED\n");

	if (options & DPNI_OPT_DIST_HASH)
		printf("\tDPNI_OPT_DIST_HASH\n");

	if (options & DPNI_OPT_DIST_FS)
		printf("\tDPNI_OPT_DIST_FS\n");

	if (options & DPNI_OPT_UNICAST_FILTER)
		printf("\tDPNI_OPT_UNICAST_FILTER\n");

	if (options & DPNI_OPT_MULTICAST_FILTER)
		printf("\tDPNI_OPT_MULTICAST_FILTER\n");

	if (options & DPNI_OPT_VLAN_FILTER)
		printf("\tDPNI_OPT_VLAN_FILTER\n");

	if (options & DPNI_OPT_IPR)
		printf("\tDPNI_OPT_IPR\n");

	if (options & DPNI_OPT_IPF)
		printf("\tDPNI_OPT_IPF\n");

	if (options & DPNI_OPT_VLAN_MANIPULATION)
		printf("\tDPNI_OPT_VLAN_MANIPULATION\n");

	if (options & DPNI_OPT_QOS_MASK_SUPPORT)
		printf("\tDPNI_OPT_QOS_MASK_SUPPORT\n");

	if (options & DPNI_OPT_FS_MASK_SUPPORT)
		printf("\tDPNI_OPT_FS_MASK_SUPPORT\n");
}

static int print_dpni_endpoint(uint32_t target_id)
{
	struct dprc_endpoint endpoint1;
	struct dprc_endpoint endpoint2;
	int state;
	int error = 0;

	memset(&endpoint1, 0, sizeof(struct dprc_endpoint));
	memset(&endpoint2, 0, sizeof(struct dprc_endpoint));

	strncpy(endpoint1.type, "dpni", EP_OBJ_TYPE_MAX_LEN);
	endpoint1.type[EP_OBJ_TYPE_MAX_LEN] = '\0';
	endpoint1.id = target_id;
	endpoint1.if_id = 0;

	error = dprc_get_connection(&restool.mc_io, 0,
					restool.root_dprc_handle,
					&endpoint1, &endpoint2, &state);
	printf("endpoint state: %d\n", state);

	if (error == 0 && state == -1) {
		printf("endpoint: No object associated\n");
	} else if (error == 0) {
		if (strcmp(endpoint2.type, "dpsw") == 0 ||
		    strcmp(endpoint2.type, "dpdmux") == 0) {
			printf("endpoint: %s.%d.%d",
				endpoint2.type, endpoint2.id,
				endpoint2.if_id);
		} else if (endpoint2.if_id == 0) {
			printf("endpoint: %s.%d",
				endpoint2.type, endpoint2.id);
		}

		if (state == 1)
			printf(", link is up\n");
		else if (state == 0)
			printf(", link is down\n");
		else
			printf(", link is in error state\n");

	} else {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			mc_status_to_string(mc_status), mc_status);
		return error;
	}

	return 0;
}

static int print_dpni_attr(uint32_t dpni_id,
			struct dprc_obj_desc *target_obj_desc)
{
	uint16_t dpni_handle;
	int error;
	struct dpni_attr dpni_attr;
	uint8_t mac_addr[6];
	bool dpni_opened = false;
	struct dpni_link_state link_state;

	error = dpni_open(&restool.mc_io, 0, dpni_id, &dpni_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	dpni_opened = true;
	if (0 == dpni_handle) {
		DEBUG_PRINTF(
			"dpni_open() returned invalid handle (auth 0) for dpni.%u\n",
			dpni_id);
		error = -ENOENT;
		goto out;
	}

	memset(&dpni_attr, 0, sizeof(dpni_attr));
	error = dpni_get_attributes(&restool.mc_io, 0, dpni_handle, &dpni_attr);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	assert(dpni_id == (uint32_t)dpni_attr.id);
	assert(DPNI_MAX_TC >= dpni_attr.max_tcs);

	error = dpni_get_primary_mac_addr(&restool.mc_io, 0,
					dpni_handle, mac_addr);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}

	memset(&link_state, 0, sizeof(link_state));
	error = dpni_get_link_state(&restool.mc_io, 0, dpni_handle,
					&link_state);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}

	printf("dpni version: %u.%u\n", dpni_attr.version.major,
	       dpni_attr.version.minor);
	printf("dpni id: %d\n", dpni_attr.id);
	printf("plugged state: %splugged\n",
		(target_obj_desc->state & DPRC_OBJ_STATE_PLUGGED) ? "" : "un");
	print_dpni_endpoint(dpni_id);
	printf("link status: %d - ", link_state.up);
	link_state.up == 0 ? printf("down\n") :
	link_state.up == 1 ? printf("up\n") : printf("error state\n");
	printf("mac address: ");
	for (int j = 0; j < 5; ++j)
		printf("%02x:", mac_addr[j]);
	printf("%02x\n", mac_addr[5]);
	printf("dpni_attr.options value is: %#lx\n",
	       (unsigned long)dpni_attr.options);
	print_dpni_options(dpni_attr.options);
	printf("max senders: %u\n", (uint32_t)dpni_attr.max_senders);
	printf("max traffic classes: %u\n", (uint32_t)dpni_attr.max_tcs);
	printf("max distribution's size per RX traffic class:\n");
	for (int k = 0; k < dpni_attr.max_tcs; ++k)
		printf("\tclass %d's size: %u\n", k,
		       (uint32_t)dpni_attr.max_dist_per_tc[k]);
	printf("max unicast filters: %u\n",
	       (uint32_t)dpni_attr.max_unicast_filters);
	printf("max multicast filters: %u\n",
	       (uint32_t)dpni_attr.max_multicast_filters);
	printf("max vlan filters: %u\n", (uint32_t)dpni_attr.max_vlan_filters);
	printf("max QoS entries: %u\n", (uint32_t)dpni_attr.max_qos_entries);
	printf("max QoS key size: %u\n", (uint32_t)dpni_attr.max_qos_key_size);
	printf("max distribution key size: %u\n",
	       (uint32_t)dpni_attr.max_dist_key_size);
	print_obj_label(target_obj_desc);

	error = 0;

out:
	if (dpni_opened) {
		int error2;

		error2 = dpni_close(&restool.mc_io, 0, dpni_handle);
		if (error2 < 0) {
			mc_status = flib_error_to_mc_status(error2);
			ERROR_PRINTF("MC error: %s (status %#x)\n",
				     mc_status_to_string(mc_status), mc_status);
			if (error == 0)
				error = error2;
		}
	}

	return error;
}

static int print_dpni_info(uint32_t dpni_id)
{
	int error;
	struct dprc_obj_desc target_obj_desc;
	uint32_t target_parent_dprc_id;
	bool found = false;

	memset(&target_obj_desc, 0, sizeof(struct dprc_obj_desc));
	error = find_target_obj_desc(restool.root_dprc_id,
				restool.root_dprc_handle, 0, dpni_id,
				"dpni", &target_obj_desc,
				&target_parent_dprc_id, &found);
	if (error < 0)
		goto out;

	if (strcmp(target_obj_desc.type, "dpni")) {
		printf("dpni.%d does not exist\n", dpni_id);
		return -EINVAL;
	}

	error = print_dpni_attr(dpni_id, &target_obj_desc);
	if (error < 0)
		goto out;

	if (restool.cmd_option_mask & ONE_BIT_MASK(INFO_OPT_VERBOSE)) {
		restool.cmd_option_mask &= ~ONE_BIT_MASK(INFO_OPT_VERBOSE);
		error = print_obj_verbose(&target_obj_desc, &dpni_ops);
	}

out:
	return error;
}

static int cmd_dpni_info(void)
{
	static const char usage_msg[] =
		"\n"
		"Usage: restool dpni info <dpni-object> [--verbose]\n"
		"   e.g. restool dpni info dpni.7\n"
		"\n"
		"--verbose\n"
		"   Shows extended/verbose information about the object\n"
		"   e.g. restool dpni info dpni.7 --verbose\n"
		"\n";

	uint32_t obj_id;
	int error;

	if (restool.cmd_option_mask & ONE_BIT_MASK(INFO_OPT_HELP)) {
		printf(usage_msg);
		restool.cmd_option_mask &= ~ONE_BIT_MASK(INFO_OPT_HELP);
		error = 0;
		goto out;
	}

	if (restool.obj_name == NULL) {
		ERROR_PRINTF("<object> argument missing\n");
		printf(usage_msg);
		error = -EINVAL;
		goto out;
	}

	error = parse_object_name(restool.obj_name, "dpni", &obj_id);
	if (error < 0)
		goto out;

	error = print_dpni_info(obj_id);
out:
	return error;
}

#define OPTION_MAP_ENTRY(_option)	{#_option, _option}

static int parse_dpni_create_options(char *options_str, uint32_t *options)
{
	static const struct {
		const char *str;
		uint32_t value;
	} options_map[] = {
		OPTION_MAP_ENTRY(DPNI_OPT_ALLOW_DIST_KEY_PER_TC),
		OPTION_MAP_ENTRY(DPNI_OPT_TX_CONF_DISABLED),
		OPTION_MAP_ENTRY(DPNI_OPT_PRIVATE_TX_CONF_ERROR_DISABLED),
		OPTION_MAP_ENTRY(DPNI_OPT_DIST_HASH),
		OPTION_MAP_ENTRY(DPNI_OPT_DIST_FS),
		OPTION_MAP_ENTRY(DPNI_OPT_UNICAST_FILTER),
		OPTION_MAP_ENTRY(DPNI_OPT_MULTICAST_FILTER),
		OPTION_MAP_ENTRY(DPNI_OPT_VLAN_FILTER),
		OPTION_MAP_ENTRY(DPNI_OPT_IPR),
		OPTION_MAP_ENTRY(DPNI_OPT_IPF),
		OPTION_MAP_ENTRY(DPNI_OPT_VLAN_MANIPULATION),
		OPTION_MAP_ENTRY(DPNI_OPT_QOS_MASK_SUPPORT),
		OPTION_MAP_ENTRY(DPNI_OPT_FS_MASK_SUPPORT),
	};

	char *cursor = NULL;
	char *opt_str = strtok_r(options_str, ", ", &cursor);
	uint32_t options_mask = 0;

	DEBUG_PRINTF("opt_str = %s\n", opt_str);

	while (opt_str != NULL) {
		unsigned int i;

		for (i = 0; i < ARRAY_SIZE(options_map); ++i) {
			if (strcmp(opt_str, options_map[i].str) == 0) {
				options_mask |= options_map[i].value;
				break;
			}
		}

		if (i == ARRAY_SIZE(options_map)) {
			ERROR_PRINTF("Invalid option: '%s'\n", opt_str);
			return -EINVAL;
		}

		opt_str = strtok_r(NULL, ", ", &cursor);
		DEBUG_PRINTF("opt_str = %s\n", opt_str);
	}

	*options = options_mask;

	return 0;
}

static int parse_dpni_mac_addr(char *mac_addr_str, uint8_t *mac_addr)
{
	char *cursor = NULL;
	char *endptr;
	char *mac_str = strtok_r(mac_addr_str, ":", &cursor);
	long val;
	int i = 0;

	while (mac_str != NULL) {
		if (i >= 6) { /* mac address is more than 6 parts */
			ERROR_PRINTF("Invalid MAC address.\n");
			ERROR_PRINTF(
				"Please enter 48 bits MAC address, eg. 00:0e:0c:55:12:03\n");
			return -EINVAL;
		}

		errno = 0;
		val = strtol(mac_str, &endptr, 16);

		if (STRTOL_ERROR(mac_str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid MAC address.\n");
			ERROR_PRINTF(
				"Please enter 48 bits MAC address, eg. 00:0e:0c:55:12:03\n");
			return -EINVAL;
		}

		mac_addr[i] = (uint8_t)val;
		mac_str = strtok_r(NULL, ":", &cursor);
		++i;
	}

	if (6 != i) { /* mac address is less than 6 parts */
		ERROR_PRINTF("Invalid MAC address.\n");
		ERROR_PRINTF(
			"Please enter 48 bits MAC address, eg. 00:0e:0c:55:12:03\n");
		return -EINVAL;
	}

	return 0;
}

static int parse_dpni_max_dist_per_tc(char *max_dist_per_tc_str,
	uint8_t *max_dist_per_tc, uint8_t max_tcs)
{
	char *cursor = NULL;
	char *endptr;
	char *max_dist_str = strtok_r(max_dist_per_tc_str, ",", &cursor);
	int i = 0;
	long val;

	while (max_dist_str != NULL) {
		if (i >= DPNI_MAX_TC) {
			ERROR_PRINTF("Invalid max-dist-per-tc\n");
			ERROR_PRINTF("maximum number of traffic class <= %d\n",
			     DPNI_MAX_TC);
			return -EINVAL;
		}

		errno = 0;
		val = strtol(max_dist_str, &endptr, 0);

		if (STRTOL_ERROR(max_dist_str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid dist-size.\n");
			return -EINVAL;
		}

		max_dist_per_tc[i] = (uint8_t)val;
		max_dist_str = strtok_r(NULL, ",", &cursor);
		++i;
	}

	if (max_tcs != i) {
		ERROR_PRINTF(
			"size of max_dist_per_tc does not match max_tcs: %u\n",
			(unsigned int)max_tcs);
		return -EINVAL;
	}

	return 0;
}

static int create_dpni(const char *usage_msg)
{
	int error;
	struct dpni_cfg dpni_cfg;
	uint16_t dpni_handle;
	long val;
	char *str;
	char *endptr;
	struct dpni_attr dpni_attr;

	memset(&dpni_cfg, 0, sizeof(dpni_cfg));

	if (restool.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_HELP)) {
		printf(usage_msg);
		restool.cmd_option_mask &= ~ONE_BIT_MASK(CREATE_OPT_HELP);
		return 0;
	}

	if (restool.obj_name != NULL) {
		ERROR_PRINTF("Unexpected argument: \'%s\'\n\n",
			     restool.obj_name);
		printf(usage_msg);
		return -EINVAL;
	}

	if (restool.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_OPTIONS)) {
		restool.cmd_option_mask &= ~ONE_BIT_MASK(CREATE_OPT_OPTIONS);
		error = parse_dpni_create_options(
				restool.cmd_option_args[CREATE_OPT_OPTIONS],
				&dpni_cfg.adv.options);
		if (error < 0) {
			DEBUG_PRINTF(
				"parse_dpni_create_options() failed with error %d, cannot get options-mask\n",
				error);
			return error;
		}
	} else {
		dpni_cfg.adv.options = DPNI_OPT_UNICAST_FILTER |
				       DPNI_OPT_MULTICAST_FILTER;
	}

	if (!(restool.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_MAC_ADDR))) {
		ERROR_PRINTF("--mac-addr option missing\n");
		printf(usage_msg);
		return -EINVAL;
	}

	restool.cmd_option_mask &= ~ONE_BIT_MASK(CREATE_OPT_MAC_ADDR);
	error  = parse_dpni_mac_addr(
			restool.cmd_option_args[CREATE_OPT_MAC_ADDR],
			dpni_cfg.mac_addr);
	if (error < 0) {
		DEBUG_PRINTF(
			"parse_dpni_mac_addr() failed with error %d, cannot get mac address\n",
			error);
		return error;
	}

	if (restool.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_MAX_TCS)) {
		restool.cmd_option_mask &= ~ONE_BIT_MASK(CREATE_OPT_MAX_TCS);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_TCS];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > DPNI_MAX_TC)) {
			ERROR_PRINTF("Invalid max tcs\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_tcs = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_tcs = 1; /* set default value 1 */
	}

	if (restool.cmd_option_mask &
	    ONE_BIT_MASK(CREATE_OPT_MAX_DIST_PER_TC)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_MAX_DIST_PER_TC);
		error = parse_dpni_max_dist_per_tc(
			restool.cmd_option_args[CREATE_OPT_MAX_DIST_PER_TC],
			dpni_cfg.adv.max_dist_per_tc,
			dpni_cfg.adv.max_tcs);
		if (error < 0) {
			DEBUG_PRINTF(
				"parse_dpni_max_dist_per_tc() failed with error %d, cannot get maximum distribution's size per RX traffic-class\n",
				error);
			return error;
		}
	} else {
		if (dpni_cfg.adv.max_tcs <= DPNI_MAX_TC) {
			for (int i = 0; i < dpni_cfg.adv.max_tcs; ++i)
				dpni_cfg.adv.max_dist_per_tc[i] = 1;
		} else {
			ERROR_PRINTF("max_tcs is out of effective range\n");
			return -EINVAL;
		}
	}

	if (restool.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_MAX_SENDERS)) {
		restool.cmd_option_mask &=
				~ONE_BIT_MASK(CREATE_OPT_MAX_SENDERS);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_SENDERS];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid max senders\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_senders = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_senders = 1;
	}

	if (restool.cmd_option_mask &
	    ONE_BIT_MASK(CREATE_OPT_MAX_UNICAST_FILTERS)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_MAX_UNICAST_FILTERS);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_UNICAST_FILTERS];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid max unicast filters\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_unicast_filters = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_unicast_filters = 0;
	}

	if (restool.cmd_option_mask &
	    ONE_BIT_MASK(CREATE_OPT_MAX_MULTICAST_FILTERS)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_MAX_MULTICAST_FILTERS);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_MULTICAST_FILTERS];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid max multicast filters\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_multicast_filters = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_multicast_filters = 0;
	}

	if (restool.cmd_option_mask &
	    ONE_BIT_MASK(CREATE_OPT_MAX_VLAN_FILTERS)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_MAX_VLAN_FILTERS);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_VLAN_FILTERS];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid max vlan filters\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_vlan_filters = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_vlan_filters = 0;
	}

	if (restool.cmd_option_mask &
	    ONE_BIT_MASK(CREATE_OPT_MAX_QOS_ENTRIES)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_MAX_QOS_ENTRIES);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_QOS_ENTRIES];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid max qos entries\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_qos_entries = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_qos_entries = 0;
	}

	if (restool.cmd_option_mask &
	    ONE_BIT_MASK(CREATE_OPT_MAX_QOS_KEY_SIZE)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_MAX_QOS_KEY_SIZE);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_QOS_KEY_SIZE];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid max qos key size\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_qos_key_size = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_qos_key_size = 0;
	}

	if (restool.cmd_option_mask &
	    ONE_BIT_MASK(CREATE_OPT_MAX_DIST_KEY_SIZE)) {
		restool.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_MAX_DIST_KEY_SIZE);
		dpni_cfg.adv.max_dist_key_size = (uint8_t)strtol(
			restool.cmd_option_args[CREATE_OPT_MAX_DIST_KEY_SIZE],
			(char **)NULL, 0);
		errno = 0;
		str = restool.cmd_option_args[CREATE_OPT_MAX_DIST_KEY_SIZE];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno) ||
		    (val < 0 || val > UINT8_MAX)) {
			ERROR_PRINTF("Invalid max dist key size\n");
			return -EINVAL;
		}

		dpni_cfg.adv.max_dist_key_size = (uint8_t)val;
	} else {
		dpni_cfg.adv.max_dist_key_size = 0;
	}

	error = dpni_create(&restool.mc_io, 0, &dpni_cfg, &dpni_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		return error;
	}

	memset(&dpni_attr, 0, sizeof(struct dpni_attr));
	error = dpni_get_attributes(&restool.mc_io, 0, dpni_handle, &dpni_attr);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		return error;
	}
	print_new_obj("dpni", dpni_attr.id, NULL);

	error = dpni_close(&restool.mc_io, 0, dpni_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		return error;
	}
	return 0;
}

static int cmd_dpni_create(void)
{
	static const char usage_msg[] =
		"\n"
		"Usage: restool dpni create --mac-addr=<addr> [OPTIONS]\n"
		"   --mac-addr=<addr>\n"
		"	String specifying primary MAC address (e.g. 00:00:05:00:00:05). It is mandatory.\n"
		"   e.g. create a DPNI object with all default options:\n"
		"	restool dpni create --mac-addr=<addr>\n"
		"\n"
		"OPTIONS:\n"
		"--max-senders=<number>\n"
		"	maximum number of different senders;\n"
		"	will be used as the number of dedicated TX flows;\n"
		"	In case it isn't power-of-2 it will be ceiling to\n"
		"	the next power-of-2 as HW demand it; 0 will be treated as 1\n"
		"--options=<options-mask>\n"
		"   Where <options-mask> is a comma or space separated list of DPNI options:\n"
		"	DPNI_OPT_ALLOW_DIST_KEY_PER_TC\n"
		"	DPNI_OPT_TX_CONF_DISABLED\n"
		"	DPNI_OPT_PRIVATE_TX_CONF_ERR_DISABLED\n"
		"	DPNI_OPT_DIST_HASH\n"
		"	DPNI_OPT_DIST_FS\n"
		"	DPNI_OPT_UNICAST_FILTER\n"
		"	DPNI_OPT_MULTICAST_FILTER\n"
		"	DPNI_OPT_VLAN_FILTER\n"
		"	DPNI_OPT_IPR\n"
		"	DPNI_OPT_IPF\n"
		"	DPNI_OPT_VLAN_MANIPULATION\n"
		"	DPNI_OPT_QOS\n"
		"--max-tcs=<number>\n"
		"	Specifies the maximum number of traffic-classes\n"
		"	0 will be treated as 1\n"
		"--max-dist-per-tc=<dist-size>,<dist-size>,…\n"
		"	Comma separated list of counts specifying the\n"
		"	maximum distribution's size per RX traffic-class\n"
		"--max-unicast-filters=<number>\n"
		"	maximum number of unicast filters;\n"
		"	0 will be treated as 16\n"
		"--max-multicast-filters=<number>\n"
		"	maximum number of multicast filters;\n"
		"	0 will be treated as 64\n"
		"--max-vlan-filters=<number>\n"
		"	maximum number of vlan filters;\n"
		"	'0' will be treated as '16'\n"
		"--max-qos-entries=<number>\n"
		"	if max_tcs > 1, declares the maximum entries for\n"
		"	the QoS table; '0' will be treated as '64'\n"
		"--max-qos-key-size=<number>\n"
		"	maximum key size for the QoS look-up;\n"
		"	'0' will be treated as '24' which enough for IPv4 5-tuple\n"
		"--max-dist-key-size=<number>\n"
		"	maximum key size for the distribution;\n"
		"	'0' will be treated as '24' which enough for IPv4 5-tuple\n"
		"\n";

	return create_dpni(usage_msg);
}

static int cmd_dpni_destroy(void)
{
	static const char usage_msg[] =
		"\n"
		"Usage: restool dpni destroy <dpni-object>\n"
		"   e.g. restool dpni destroy dpni.9\n"
		"\n";

	int error;
	int error2;
	uint32_t dpni_id;
	uint16_t dpni_handle;
	bool dpni_opened = false;


	if (restool.cmd_option_mask & ONE_BIT_MASK(DESTROY_OPT_HELP)) {
		printf(usage_msg);
		restool.cmd_option_mask &= ~ONE_BIT_MASK(DESTROY_OPT_HELP);
		return 0;
	}

	if (restool.obj_name == NULL) {
		ERROR_PRINTF("<object> argument missing\n");
		printf(usage_msg);
		error = -EINVAL;
		goto out;
	}

	if (in_use(restool.obj_name, "destroyed")) {
		error = -EBUSY;
		goto out;
	}

	error = parse_object_name(restool.obj_name, "dpni", &dpni_id);
	if (error < 0)
		goto out;

	if (!find_obj("dpni", dpni_id)) {
		error = -EINVAL;
		goto out;
	}

	error = dpni_open(&restool.mc_io, 0, dpni_id, &dpni_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	dpni_opened = true;
	if (0 == dpni_handle) {
		DEBUG_PRINTF(
			"dpni_open() returned invalid handle (auth 0) for dpni.%u\n",
			dpni_id);
		error = -ENOENT;
		goto out;
	}

	error = dpni_destroy(&restool.mc_io, 0, dpni_handle);
	if (error < 0) {
		mc_status = flib_error_to_mc_status(error);
		ERROR_PRINTF("MC error: %s (status %#x)\n",
			     mc_status_to_string(mc_status), mc_status);
		goto out;
	}
	dpni_opened = false;
	printf("dpni.%u is destroyed\n", dpni_id);

out:
	if (dpni_opened) {
		error2 = dpni_close(&restool.mc_io, 0, dpni_handle);
		if (error2 < 0) {
			mc_status = flib_error_to_mc_status(error2);
			ERROR_PRINTF("MC error: %s (status %#x)\n",
				     mc_status_to_string(mc_status), mc_status);
			if (error == 0)
				error = error2;
		}
	}

	return error;
}

struct object_command dpni_commands[] = {
	{ .cmd_name = "help",
	  .options = NULL,
	  .cmd_func = cmd_dpni_help },

	{ .cmd_name = "info",
	  .options = dpni_info_options,
	  .cmd_func = cmd_dpni_info },

	{ .cmd_name = "create",
	  .options = dpni_create_options,
	  .cmd_func = cmd_dpni_create },

	{ .cmd_name = "destroy",
	  .options = dpni_destroy_options,
	  .cmd_func = cmd_dpni_destroy },

	{ .cmd_name = NULL },
};

