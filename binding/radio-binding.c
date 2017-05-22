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

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

#include <json-c/json.h>
#include <afb/afb-binding.h>
#include <afb/afb-service-itf.h>

#include "radio_impl.h"

static const struct afb_binding_interface *interface;

static struct afb_event freq_event;
static struct afb_event scan_event;

static void freq_callback(uint32_t frequency, void *data)
{
	json_object *jresp = json_object_new_object();
	json_object *value = json_object_new_int((int) frequency);

	json_object_object_add(jresp, "value", value);
	afb_event_push(freq_event, json_object_get(jresp));
}

static void scan_callback(uint32_t frequency, void *data)
{
	json_object *jresp = json_object_new_object();
	json_object *value = json_object_new_int((int) frequency);

	json_object_object_add(jresp, "value", value);
	afb_event_push(scan_event, json_object_get(jresp));
}

/*
 * Binding verb handlers
 */

/*
 * @brief Get (and optionally set) frequency
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void frequency(struct afb_req request)
{
	json_object *ret_json;
	const char *value = afb_req_value(request, "value");
	uint32_t frequency;

	if(value) {
		char *p;
		frequency = strtoul(value, &p, 10);
		if(frequency && *p == '\0') {
			radio_impl_set_frequency(frequency);
		} else {
			afb_req_fail(request, "failed", "Invalid scan direction");
			return;
		}
	}
	ret_json = json_object_new_object();
	frequency = radio_impl_get_frequency();
	json_object_object_add(ret_json, "frequency", json_object_new_int((int32_t) frequency));
	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Get (and optionally set) frequency band
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void band(struct afb_req request)
{
	json_object *ret_json;
	const char *value = afb_req_value(request, "value");
	int valid = 0;
	radio_band_t band;
	char band_name[4];

	if(value) {
		if(!strcasecmp(value, "AM")) {
			band = BAND_AM;
			valid = 1;
		} else if(!strcasecmp(value, "FM")) {
			band = BAND_FM;
			valid = 1;
		} else {
			char *p;
			band = strtoul(value, &p, 10);
			if(p != value && *p == '\0') {
				switch(band) {
				case BAND_AM:
				case BAND_FM:
					valid = 1;
					break;
				default:
					break;
				}
			}
		}
		if(valid) {
			radio_impl_set_band(band);
		} else {
			afb_req_fail(request, "failed", "Invalid band");
			return;
		}
	}
	ret_json = json_object_new_object();
	band = radio_impl_get_band();
	sprintf(band_name, "%s", band == BAND_AM ? "AM" : "FM");
	json_object_object_add(ret_json, "band", json_object_new_string(band_name));
	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Check if band is supported
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void band_supported(struct afb_req request)
{
	json_object *ret_json;
	const char *value = afb_req_value(request, "band");
	int valid = 0;
	radio_band_t band;

	if(value) {
		if(!strcasecmp(value, "AM")) {
			band = BAND_AM;
			valid = 1;
		} else if(!strcasecmp(value, "FM")) {
			band = BAND_FM;
			valid = 1;
		} else {
			char *p;
			band = strtoul(value, &p, 10);
			if(p != value && *p == '\0') {
				switch(band) {
				case BAND_AM:
				case BAND_FM:
					valid = 1;
					break;
				default:
					break;
				}
			}
		}
	}
	if(!valid) {
		afb_req_fail(request, "failed", "Invalid band");
		return;
	}
	ret_json = json_object_new_object();
	json_object_object_add(ret_json,
			       "supported",
			       json_object_new_int(radio_impl_band_supported(band)));
	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Get frequency range for a band
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void frequency_range(struct afb_req request)
{
	json_object *ret_json;
	const char *value = afb_req_value(request, "band");
	int valid = 0;
	radio_band_t band;
	uint32_t min_frequency;
	uint32_t max_frequency;

	if(value) {
		if(!strcasecmp(value, "AM")) {
			band = BAND_AM;
			valid = 1;
		} else if(!strcasecmp(value, "FM")) {
			band = BAND_FM;
			valid = 1;
		} else {
			char *p;
			band = strtoul(value, &p, 10);
			if(p != value && *p == '\0') {
				switch(band) {
				case BAND_AM:
				case BAND_FM:
					valid = 1;
					break;
				default:
					break;
				}
			}
		}
	}
	if(!valid) {
		afb_req_fail(request, "failed", "Invalid band");
		return;
	}
	ret_json = json_object_new_object();
	min_frequency = radio_impl_get_min_frequency(band);
	max_frequency = radio_impl_get_max_frequency(band);
	json_object_object_add(ret_json, "min", json_object_new_int((int32_t) min_frequency));
	json_object_object_add(ret_json, "max", json_object_new_int((int32_t) max_frequency));
	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Get frequency step size (Hz) for a band
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void frequency_step(struct afb_req request)
{
	json_object *ret_json;
	const char *value = afb_req_value(request, "band");
	int valid = 0;
	radio_band_t band;
	uint32_t step;

	if(value) {
		if(!strcasecmp(value, "AM")) {
			band = BAND_AM;
			valid = 1;
		} else if(!strcasecmp(value, "FM")) {
			band = BAND_FM;
			valid = 1;
		} else {
			char *p;
			band = strtoul(value, &p, 10);
			if(p != value && *p == '\0') {
				switch(band) {
				case BAND_AM:
				case BAND_FM:
					valid = 1;
					break;
				default:
					break;
				}
			}
		}
	}
	if(!valid) {
		afb_req_fail(request, "failed", "Invalid band");
		return;
	}
	ret_json = json_object_new_object();
	step = radio_impl_get_frequency_step(band);
	json_object_object_add(ret_json, "step", json_object_new_int((int32_t) step));
	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Start radio playback
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void start(struct afb_req request)
{
	radio_impl_start();
	afb_req_success(request, NULL, NULL);
}

/*
 * @brief Stop radio playback
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void stop(struct afb_req request)
{
	radio_impl_stop();
	afb_req_success(request, NULL, NULL);
}

/*
 * @brief Scan for a station in the specified direction
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void scan_start(struct afb_req request)
{
	json_object *ret_json;
	const char *value = afb_req_value(request, "direction");
	int valid = 0;
	radio_scan_direction_t direction;

	if(value) {
		if(!strcasecmp(value, "forward")) {
			direction = SCAN_FORWARD;
			valid = 1;
		} else if(!strcasecmp(value, "backward")) {
			direction = SCAN_BACKWARD;
			valid = 1;
		} else {
			char *p;
			direction = strtoul(value, &p, 10);
			if(p != value && *p == '\0') {
				switch(direction) {
				case SCAN_FORWARD:
				case SCAN_BACKWARD:
					valid = 1;
					break;
				default:
					break;
				}
			}
		}
	}
	if(!valid) {
		afb_req_fail(request, "failed", "Invalid direction");
		return;
	}
	radio_impl_scan_start(direction, scan_callback, NULL);
	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Stop station scan
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void scan_stop(struct afb_req request)
{
	radio_impl_scan_stop();
	afb_req_success(request, NULL, NULL);
}

/*
 * @brief Get (and optionally set) stereo mode
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void stereo_mode(struct afb_req request)
{
	json_object *ret_json;
	const char *value = afb_req_value(request, "value");
	int valid = 0;
	radio_stereo_mode_t mode;
	char mode_name[4];

	if(value) {
		if(!strcasecmp(value, "mono")) {
			mode = MONO;
			valid = 1;
		} else if(!strcasecmp(value, "stereo")) {
			mode = STEREO;
			valid = 1;
		} else {
			char *p;
			mode = strtoul(value, &p, 10);
			if(p != value && *p == '\0') {
				switch(mode) {
				case MONO:
				case STEREO:
					valid = 1;
					break;
				default:
					break;
				}
			}
		}
		if(valid) {
			radio_impl_set_stereo_mode(mode);
		} else {
			afb_req_fail(request, "failed", "Invalid mode");
			return;
		}
	}
	ret_json = json_object_new_object();
	mode = radio_impl_get_stereo_mode();
	sprintf(mode_name, "%s", mode == MONO ? "mono" : "stereo");
	json_object_object_add(ret_json, "mode", json_object_new_string(mode_name));
	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Subscribe for an event
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void subscribe(struct afb_req request)
{
	const char *value = afb_req_value(request, "value");
	if(value) {
		if(!strcasecmp(value, "frequency")) {
			afb_req_subscribe(request, freq_event);
		} else if(!strcasecmp(value, "station_found")) {
			afb_req_subscribe(request, scan_event);
		} else {
			afb_req_fail(request, "failed", "Invalid event");
			return;
		}
	}
	afb_req_success(request, NULL, NULL);
}

/*
 * @brief Unsubscribe for an event
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void unsubscribe(struct afb_req request)
{
	const char *value = afb_req_value(request, "value");
	if(value) {
		if(!strcasecmp(value, "frequency")) {
			afb_req_unsubscribe(request, freq_event);
		} else if(!strcasecmp(value, "station_found")) {
			afb_req_unsubscribe(request, scan_event);
		} else {
			afb_req_fail(request, "failed", "Invalid event");
			return;
		}
	}
	afb_req_success(request, NULL, NULL);
}

static const struct afb_verb_desc_v1 verbs[]= {
	{ "frequency",		AFB_SESSION_CHECK, frequency,		"Get/Set frequency" },
	{ "band",		AFB_SESSION_CHECK, band,		"Get/Set band" },
	{ "band_supported",	AFB_SESSION_CHECK, band_supported,	"Check band support" },
	{ "frequency_range",	AFB_SESSION_CHECK, frequency_range,	"Get frequency range" },
	{ "frequency_step",	AFB_SESSION_CHECK, frequency_step,	"Get frequency step" },
	{ "start",		AFB_SESSION_CHECK, start,		"Start radio playback" },
	{ "stop",		AFB_SESSION_CHECK, stop,		"Stop radio playback" },
	{ "scan_start",		AFB_SESSION_CHECK, scan_start,		"Start station scan" },
	{ "scan_stop",		AFB_SESSION_CHECK, scan_stop,		"Stop station scan" },
	{ "stereo_mode",	AFB_SESSION_CHECK, stereo_mode,		"Get/Set stereo_mode" },
	{ "subscribe",		AFB_SESSION_CHECK, subscribe,		"Subscribe for an event" },
	{ "unsubscribe",	AFB_SESSION_CHECK, unsubscribe,		"Unsubscribe for an event" },
	{ NULL }
};

static const struct afb_binding binding_desc = {
	.type = AFB_BINDING_VERSION_1,
	.v1 = {
		.info = "radio service",
		.prefix = "radio",
		.verbs = verbs
	}
};

const struct afb_binding *afbBindingV1Register (const struct afb_binding_interface *itf)
{
	interface = itf;

	return &binding_desc;
}

int afbBindingV1ServiceInit(struct afb_service service)
{
	int rc;

	freq_event = afb_daemon_make_event(interface->daemon, "frequency");
	scan_event = afb_daemon_make_event(interface->daemon, "station_found");

	rc = radio_impl_init();
	if(rc == 0) {
		radio_impl_set_frequency_callback(freq_callback, NULL);
	}
	
	return rc;
}
