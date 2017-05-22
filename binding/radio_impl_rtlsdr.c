/*
 * Copyright (C) 2017 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <glib.h>

#include "radio_impl.h"
#include "radio_output.h"
#include "rtl_fm.h"

// Structure to describe FM band plans, all values in Hz.
typedef struct {
	char *name;
	uint32_t min;
	uint32_t max;
	uint32_t step;
} fm_band_plan_t;

static fm_band_plan_t known_fm_band_plans[5] = {
	{ .name = "US", .min = 87900000, .max = 107900000, .step = 200000 },
	{ .name = "JP", .min = 76100000, .max = 89900000, .step = 100000 },
	{ .name = "EU", .min = 87500000, .max = 108000000, .step = 50000 },
	{ .name = "ITU-1", .min = 87500000, .max = 108000000, .step = 50000 },
	{ .name = "ITU-2", .min = 87900000, .max = 107900000, .step = 50000 }
};

static unsigned int bandplan;
static bool present;
static bool active;
static uint32_t current_frequency;

static void rtl_output_callback(int16_t *result, int result_len, void *ctx)
{
    if(active)
	    radio_output_write((char*) result, result_len * 2);
}

int radio_impl_init(void)
{
	GKeyFile* conf_file;
	int conf_file_present = 0;
	char *value_str;

	if(present)
		return -1;

	// Load settings from configuration file if it exists
	conf_file = g_key_file_new();
	if(conf_file &&
	   g_key_file_load_from_dirs(conf_file,
				     "AGL.conf",
				     (const gchar**) g_get_system_config_dirs(),
				     NULL,
				     G_KEY_FILE_KEEP_COMMENTS,
				     NULL) == TRUE) {
		conf_file_present = 1;

		// Set band plan if it is specified
		value_str = g_key_file_get_string(conf_file,
						  "radio",
						  "fmbandplan",
						  NULL);
		if(value_str) {
			unsigned int i;
			for(i = 0;
			    i < sizeof(known_fm_band_plans) / sizeof(fm_band_plan_t);
			    i++) {
				if(!strcasecmp(value_str, known_fm_band_plans[i].name)) {
					bandplan = i;
					break;
				}
			}
		}
	}
	fprintf(stderr, "Using FM Bandplan: %s\n", known_fm_band_plans[bandplan].name);

	current_frequency = radio_impl_get_min_frequency(BAND_FM);
	if(rtl_fm_init(current_frequency, 200000, 48000, rtl_output_callback, NULL) < 0) {
		return -1;
	}

	if(conf_file_present) {
		GError *error = NULL;
		int n;

		// Allow over-riding scanning parameters just in case a demo
		// setup needs to do so to work reliably.
		n = g_key_file_get_integer(conf_file,
					   "radio",
					   "scan_squelch_level",
					   &error);
		//error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND &&
		//error->code != G_KEY_FILE_ERROR_INVALID_VALUE) {
		if(!error) {
			fprintf(stderr, "Scanning squelch level set to %d\n", n);
			rtl_fm_scan_set_squelch_level(n);
		}

		error = NULL;
		n = g_key_file_get_integer(conf_file,
					   "radio",
					   "scan_squelch_limit",
					   &error);
		if(!error) {
			fprintf(stderr, "Scanning squelch limit set to %d\n", n);
			rtl_fm_scan_set_squelch_limit(n);
		}

		g_key_file_free(conf_file);
	}

	present = true;
	return 0;
}

uint32_t radio_impl_get_frequency(void)
{
	return current_frequency;
}

void radio_impl_set_frequency(uint32_t frequency)
{
	if(!present)
		return;

	if(frequency < known_fm_band_plans[bandplan].min ||
	   frequency > known_fm_band_plans[bandplan].max)
		return;

	radio_impl_scan_stop();
	current_frequency = frequency;
	rtl_fm_set_freq(frequency);
}

void radio_impl_set_frequency_callback(radio_freq_callback_t callback,
				       void *data)
{
	rtl_fm_set_freq_callback(callback, data);
}

radio_band_t radio_impl_get_band(void)
{
	return BAND_FM;
}

void radio_impl_set_band(radio_band_t band)
{
	// We only support FM, so do nothing
}

int radio_impl_band_supported(radio_band_t band)
{
	if(band == BAND_FM)
		return 1;
	return 0;
}

uint32_t radio_impl_get_min_frequency(radio_band_t band)
{
	return known_fm_band_plans[bandplan].min;
}

uint32_t radio_impl_get_max_frequency(radio_band_t band)
{
	return known_fm_band_plans[bandplan].max;
}

uint32_t radio_impl_get_frequency_step(radio_band_t band)
{
	uint32_t ret = 0;

	switch (band) {
	case BAND_AM:
		ret = 1000; // 1 kHz
		break;
	case BAND_FM:
		ret = known_fm_band_plans[bandplan].step;
		break;
	default:
		break;
	}
	return ret;
}

void radio_impl_start(void)
{
	if(!present)
		return;

	if(!active) {
		if(radio_output_start() != 0)
			return;

		rtl_fm_start();
		active = true;
	}
}

void radio_impl_stop(void)
{
	if(!present)
		return;

	if(active) {
		active = false;
		radio_output_stop();
		rtl_fm_stop();

	}
}

void radio_impl_scan_start(radio_scan_direction_t direction,
			   radio_scan_callback_t callback,
			   void *data)
{
	rtl_fm_scan_start(direction == SCAN_FORWARD ? 0 : 1,
			  callback,
			  data,
			  radio_impl_get_frequency_step(BAND_FM),
			  radio_impl_get_min_frequency(BAND_FM),
			  radio_impl_get_max_frequency(BAND_FM));
}

void radio_impl_scan_stop(void)
{
	rtl_fm_scan_stop();
}

radio_stereo_mode_t radio_impl_get_stereo_mode(void)
{
	return STEREO;
}

void radio_impl_set_stereo_mode(radio_stereo_mode_t mode)
{
	// We only support stereo, so do nothing
}
