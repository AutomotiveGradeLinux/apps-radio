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

#ifndef _RADIO_IMPL_H
#define _RADIO_IMPL_H

#include <stdint.h>

typedef enum {
	BAND_AM = 0,
	BAND_FM
} radio_band_t;

typedef enum {
	SCAN_FORWARD = 0,
	SCAN_BACKWARD
} radio_scan_direction_t;

typedef void (*radio_scan_callback_t)(uint32_t frequency, void *data);

typedef void (*radio_freq_callback_t)(uint32_t frequency, void *data);

typedef enum {
	MONO = 0,
	STEREO
} radio_stereo_mode_t;

int radio_impl_init(void);

uint32_t radio_impl_get_frequency(void);

void radio_impl_set_frequency(uint32_t frequency);

void radio_impl_set_frequency_callback(radio_freq_callback_t callback,
				       void *data);

radio_band_t radio_impl_get_band(void);

void radio_impl_set_band(radio_band_t band);

int radio_impl_band_supported(radio_band_t band);

uint32_t radio_impl_get_min_frequency(radio_band_t band);

uint32_t radio_impl_get_max_frequency(radio_band_t band);

uint32_t radio_impl_get_frequency_step(radio_band_t band);

void radio_impl_start(void);

void radio_impl_stop(void);

void radio_impl_scan_start(radio_scan_direction_t direction,
			   radio_scan_callback_t callback,
			   void *data);

void radio_impl_scan_stop(void);

radio_stereo_mode_t radio_impl_get_stereo_mode(void);

void radio_impl_set_stereo_mode(radio_stereo_mode_t mode);

#endif /* _RADIO_IMPL_H */
