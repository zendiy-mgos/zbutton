/*
 * Copyright (c) 2020 ZenDIY
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MGOS_ZBUTTON_H_
#define MGOS_ZBUTTON_H_

#include <stdio.h>
#include <stdbool.h>
#include "mgos_event.h"
#include "mgos_zthing.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MGOS_ZTHING_BUTTON (8 | MGOS_ZTHING_SENSOR)

#define MGOS_ZBUTTON_BASE \
  MGOS_ZTHING_BASE

struct mgos_zbutton {
  MGOS_ZBUTTON_BASE
};

struct mg_zbutton;

#define MGOS_ZBUTTON_CAST(h) ((struct mgos_zbutton *)h)

#define MGOS_ZBUTTON_DEFAULT_CLICK_TICKS 140 //milliseconds
#define MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS 160 //milliseconds
#define MGOS_ZBUTTON_DEFAULT_PRESS_TICKS 1500 //1.5 seconds
#define MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT 15000 //15 seconds

#define MGOS_ZBUTTON_CFG {                    \
  MGOS_ZBUTTON_DEFAULT_CLICK_TICKS,           \
  MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS,  \
  MGOS_ZBUTTON_DEFAULT_PRESS_TICKS,           \
  MGOS_ZBUTTON_DEFAULT_PRESS_TICKS,           \
  MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT }

struct mgos_zbutton_cfg {
  int click_ticks;
  int dblclick_delay_ticks;
  int press_ticks;
  int press_repeat_ticks;
  int press_timeout;
};

#define MGOS_ZBUTTON_EVENT_BASE MGOS_EVENT_BASE('Z', 'B', 'N')

enum mgos_zbutton_event {
  MGOS_EV_ZBUTTON_DOWN = MGOS_ZBUTTON_EVENT_BASE,
  MGOS_EV_ZBUTTON_UP,
  MGOS_EV_ZBUTTON_ON_CLICK,
  MGOS_EV_ZBUTTON_ON_DBLCLICK,
  MGOS_EV_ZBUTTON_ON_PRESS
};

struct mgos_zbutton *mgos_zbutton_create(const char *id,
                                         struct mgos_zbutton_cfg *cfg);

bool mgos_zbutton_is_pressed(struct mgos_zbutton *handle);

int mgos_zbutton_press_duration_get(struct mgos_zbutton *handle);

int mgos_zbutton_press_counter_get(struct mgos_zbutton *handle);

void mgos_zbutton_reset(struct mgos_zbutton *handle);

void mgos_zbutton_close(struct mgos_zbutton *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MGOS_ZBUTTON_H_ */