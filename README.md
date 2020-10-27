# ZenButton
## Overview
Mongoose-OS library for ZenButtons ecosystem.

## GET STARTED
Build up your own device in few minutes just starting from the following sample. Start including following libraries into your `mos.yml` file.
```yaml
libs:
  - origin: https://github.com/zendiy-mgos/zbutton-gpio
```
**C/C++ sample code**
```c
#include "mgos.h"
#include "mgos_zbutton_gpio.h"

void mg_btn_on_event_cb(int ev, void *ev_data, void *ud) {
  struct mgos_zbutton *handle = (struct mgos_zbutton *)ev_data;
  if (handle) {
    if (ev == MGOS_EV_ZBUTTON_ON_CLICK) {
      // Do something here...
      LOG(LL_INFO, ("Button '%s' CLICKED", handle->id));
    } else if (ev == MGOS_EV_ZBUTTON_ON_DBLCLICK) {
      // Do something here...
      LOG(LL_INFO, ("Button '%s' DOUBLE-CLICKED", handle->id));
    } else if (ev == MGOS_EV_ZBUTTON_ON_PRESS) {
      // Do something here...
      LOG(LL_INFO, ("Button '%s' PRESSED (count #%d)", handle->id,
        mgos_zbutton_press_counter_get(handle)));
    } else if (ev == MGOS_EV_ZBUTTON_ON_PRESS_END) {
      // Do something here...
      LOG(LL_INFO, ("Button '%s' RELEASED (pressed for %dms)", handle->id,
        mgos_zbutton_press_duration_get(handle)));
    }
  }
  (void) ud;
}

enum mgos_app_init_result mgos_app_init(void) {
  /* Create button using defualt configuration. */
  struct mgos_zbutton_cfg cfg = MGOS_ZBUTTON_CFG;
  struct mgos_zbutton *btn1 = mgos_zbutton_create("btn1", &cfg);
  
  if (btn1) {
    /* Attach button to GPIO 14. */
    struct mgos_zbutton_gpio_cfg gpio_cfg = MGOS_ZBUTTON_GPIO_CFG;  
    if (mgos_zbutton_gpio_attach(btn1, 14, &gpio_cfg)) {
      if (mgos_event_add_group_handler(MGOS_EV_ZBUTTON_ON_ANY, mg_btn_on_event_cb, NULL)) {
        return MGOS_APP_INIT_SUCCESS;
      }
      mgos_zbutton_gpio_detach(btn1);
    }
    mgos_zbutton_close(btn1);
  }
  return MGOS_APP_INIT_ERROR;
}
```
**JavaScript sample code**

```js
load("api_events.js")
load("api_zbutton_gpio.js")

function onBtnEvent(ev, evdata, ud) {
  let btn = ZenThing.getFromHandle(evdata);
  if (ev === ZenButton.EV_ON_CLICK) {
    // Do something here...
    print("Button", btn.id, "CLICKED");
  } else if (ev === ZenButton.EV_ON_DBLCLICK) {
    // Do something here...
    print("Button", btn.id, "DOUBLE-CLICKED");
  } else if (ev === ZenButton.EV_ON_PRESS) {
    // Do something here...
    print("Button", btn.id, "PRESSED", btn.getPressCounter());
  } else if (ev === ZenButton.EV_ON_PRESS_END) {
    // Do something here...
    print("Button", btn.id, "RELESED after", btn.getPressDuration(), "(ms)");
  }
}

/* Create button using defualt configuration. */
let btn1 = ZenButton.create('btn1');

if (btn1) {
  /* Attach button to GPIO 14. */
  if (!btn1.GPIO.attach(14)) {
    btn1.close();
  } else {
    Event.addGroupHandler(ZenButton.EV_ON_ANY, onBtnEvent, null);
  }
}
```
## C/C++ API Reference
### mgos_zbutton_event
```c
enum mgos_zbutton_event {
  MGOS_EV_ZBUTTON_ON_ANY,
  MGOS_EV_ZBUTTON_ON_DOWN,
  MGOS_EV_ZBUTTON_ON_UP,
  MGOS_EV_ZBUTTON_ON_CLICK,
  MGOS_EV_ZBUTTON_ON_DBLCLICK,
  MGOS_EV_ZBUTTON_ON_PRESS,
  MGOS_EV_ZBUTTON_ON_PRESS_END
};
```
A button instance publishes following events, so you can subcribe to them using `mgos_event_add_handler()` or `mgos_event_add_group_handler()`.
|Event||
|--|--|
|MGOS_EV_ZBUTTON_ON_ANY|Subscribe to this event using `mgos_event_add_group_handler()` for listening to all events.|
|MGOS_EV_ZBUTTON_ON_CLICK|Published when the button is clicked (single-click).|
|MGOS_EV_ZBUTTON_ON_DBLCLICK|Published when the button is double-clicked.|
|MGOS_EV_ZBUTTON_ON_PRESS|Published when the button is long-pressed.|
|MGOS_EV_ZBUTTON_ON_PRESS_END|Published when the button is not long-pressed anymore.|

If you are not using a driver library like [zbutton-gpio](https://github.com/zendiy-mgos/zbutton-gpio), you must notify to the button instance status changes of the physical button. You can do that sending proper events to the button instance.
|Event||
|--|--|
|MGOS_EV_ZBUTTON_ON_DOWN|Send this event when the physical button is pushed down.|
|MGOS_EV_ZBUTTON_ON_UP|Send this event when the physical button is released.|

```c
// Example: send events to the button instance according the
// status of the physical button connected to the GPIO 14

#include "mgos.h"
#include "mgos_gpio.h"
#include "mgos_zbutton.h"

void mg_zbutton_gpio_button_handler_cb(int pin, void *arg) {
  struct mgos_zbutton *handle = (struct mgos_zbutton *)arg;
  bool gpio_val = mgos_gpio_read(pin);  
  mgos_event_trigger(gpio_val ? MGOS_EV_ZBUTTON_ON_DOWN : MGOS_EV_ZBUTTON_ON_UP, handle);
  LOG(LL_DEBUG, ("Triggering button %s on pin %d ('%s').", gpio_val ? "DOWN" : "UP", pin, handle->id));
}

/* Create button using defualt configuration. */
struct mgos_zbutton_cfg cfg = MGOS_ZBUTTON_CFG;
struct mgos_zbutton *btn = mgos_zbutton_create("btn1", &cfg);

mgos_gpio_set_button_handler(14, MGOS_GPIO_PULL_DOWN,
  MGOS_GPIO_INT_EDGE_ANY, cfg.debounce_ticks,
  mg_zbutton_gpio_button_handler_cb, btn);
```
### mgos_zbutton
```c
struct mgos_zbutton {
  char *id;
  int type;
};
```
Button handle. You can get a valid handle using `mgos_zbutton_create()`.

|Field||
|--|--|
|id|Handle unique ID.|
|type|Handle type. Fixed value: `MGOS_ZTHING_BUTTON`.|

**Example** - Use of handle fields.
```c
struct mgos_zbutton *handle = mgos_zbutton_create("btn-1", NULL);
LOG(LL_INFO, ("ID '%s' detected.", handle->id));
if ((handle->type & MGOS_ZTHING_SENSOR) == MGOS_ZTHING_SENSOR) {
  LOG(LL_INFO, ("Sensor's handle detected."));
}
if (handle->type == MGOS_ZTHING_BUTTON) {
  LOG(LL_INFO, ("Button's handle detected."));
}
```
The console output:
```console
ID 'btn-1' detected.
Sensor's handle detected.
Button's handle detected.
```
### mgos_zbutton_cfg
```c
struct mgos_zbutton_cfg {
  int click_ticks;
  int press_ticks;
  int press_repeat_ticks;
  int debounce_ticks;
};
```
Button configuration values (e.g.: used by `mgos_zbutton_create()`).

|Field||
|--|--|
|click_ticks|Single click duration, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_CLICK_TICKS` to use the default duration (600ms).|
|press_ticks|Long-press duration, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS ` to use the default duration (1s).|
|press_repeat_ticks|Interval in milliseconds, for raising multiple `MGOS_EV_ZBUTTON_ON_PRESS` events, subsequent to the first one. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS` to use the default interval (1s). Set to `0` to disable event repetition.|
|debounce_ticks|Debounce interval in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_DEBOUNCE_TICKS` to use the default timeout (50ms). Set to `0` to disable it.|

**Example** - Create and initialize configuration settings.
```c
// create and initialize cfg using defaults
struct mgos_zbutton_cfg cfg = MGOS_ZBUTTON_CFG;
```
### mgos_zbutton_create()
```c
struct mgos_zbutton *mgos_zbutton_create(const char *id, struct mgos_zbutton_cfg *cfg);
```
Creates and initializes the button instance. Returns the instance handle, or `NULL` on error.

|Parameter||
|--|--|
|id|Unique ZenThing ID.|
|cfg|Optional. [Button configuration](https://github.com/zendiy-mgos/zbutton#mgos_zbutton_cfg). If `NULL`, default configuration values are used.|
### mgos_zbutton_cfg_get()
```c
void mgos_zbutton_cfg_get(struct mgos_zbutton *handle, struct mgos_zbutton_cfg *cfg);
```
Returns current button's configuration.

|Parameter||
|--|--|
|handle|Button handle.|
|cfg|Current button's configuration as output.|
### mgos_zbutton_close()
```c
void mgos_zbutton_close(struct mgos_zbutton *handle);
```
Closes and destroys the button instance.

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_reset()
```c
void mgos_zbutton_reset(struct mgos_zbutton *handle);
```
Resets button's state.

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_is_pressed()
```c
bool mgos_zbutton_is_pressed(struct mgos_zbutton *handle);
```
Returns `true` if the button is pressed (long-press) or if it was released after a long-press, otherwise `false`.

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_press_duration_get()
```c
int mgos_zbutton_press_duration_get(struct mgos_zbutton *handle);
```
Returns how long the button has been long-pressed, in milliseconds. Returns `0` if the button is not long-pressed anymore.

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_press_counter_get()
```c
int mgos_zbutton_press_counter_get(struct mgos_zbutton *handle);
```
Returns `0` if the button is not long-pressed. Otherwise, it returns `1` or the counter since the button has been long-pressed. The counter is increased every `press_repeat_ticks` milliseconds if value greater than `0` was provided (see [configuration properties](https://github.com/zendiy-mgos/zbutton#mgos_zbutton_cfg).

|Parameter||
|--|--|
|handle|Button handle.|
## JS API Reference
### ZenButton events
```js
ZenButton.EV_ON_ANY
ZenButton.EV_ON_DOWN
ZenButton.EV_ON_UP
ZenButton.EV_ON_CLICK
ZenButton.EV_ON_DBLCLICK
ZenButton.EV_ON_PRESS
ZenButton.EV_ON_PRESS_END
```
A button instance publishes following events, so you can subcribe to them using `Event.addHandler()` or `Event.addGroupHandler()`.
|Event||
|--|--|
|EV_ON_ANY|Subscribe to this event using `Event.addGroupHandler()` for listening to all events.|
|EV_ON_CLICK|Published when the button is clicked (single-click).|
|EV_ON_DBLCLICK|Published when the button is double-clicked.|
|EV_ON_PRESS|Published when the button is long-pressed.|
|EV_ON_PRESS_END|Published when the button is not long-pressed anymore.|

If you are not using a driver library like [zbutton-gpio](https://github.com/zendiy-mgos/zbutton-gpio), you must notify to the button instance status changes of the physical button. You can do that sending proper events to the button instance.
|Event||
|--|--|
|EV_ON_DOWN|Send this event when the physical button is pushed down.|
|EV_ON_UP|Send this event when the physical button is released.|
### ZenButton.create()
```js
let btn = ZenButton.create(id, cfg);
```
Creates and initializes the switch instance. Returns the instance, or `null` on error.

|Parameter|Type||
|--|--|--|
|id|string|Unique ZenThing ID.|
|cfg|object|Optional. Button configuration. If missing, default configuration values are used. For more details see 'Button configuration properties' below.<br><br>{&nbsp;clickTicks: 600,<br>&nbsp;&nbsp;pressTicks: 1000,<br>&nbsp;&nbsp;pressRepeatTicks: 1000,<br>&nbsp;&nbsp;debounceTicks: 50&nbsp;}|

<a name="js_zbutton_cfg"></a>**Button configuration properties**
|Property|Type||
|--|--|--|
|clickTicks|numeric|Optional. Single click duration, in milliseconds. Default value 600ms.|
|pressTicks|numeric|Optional. Long-press duration, in milliseconds. Default value 1s.|
|pressRepeatTicks|numeric|Optional. Interval in milliseconds, for raising multiple `ZenButton.EV_ON_PRESS` events, subsequent to the first one. Set to `0` to disable event repetition. Default value 1s.|
|debounceTicks|numeric|Optional. Debounce interval in milliseconds. Set to `0` to disable it. Default value 50ms.|

**Button instance properties** - The created instance has following properties.
|Property|Type||
|--|--|--|
|id|string|Instance ID.|
|type|numeric|Instance type. Fixed value: `ZenButton.THING_TYPE`.|

**Example 1** - Create a button using default configuration values.
```js
let btn = ZenButton.create('btn-1');
```
### .close()
```js
btn.close();
```
Closes and destroys the button instance.
### .reset()
```js
btn.reset();
```
Resets button's state.
### .isPressed()
```js
let pressed = btn.isPressed();
```
Returns `true` if the button has been long-pressed or if it was released after a long-press, otherwise `false`.
### .getPressDuration()
```js
let duration = btn.getPressDuration(); //milliseconds
```
Returns how long the button has been long-pressed, in milliseconds. Returns `0` if the button is not long-pressed anymore.
### .getPressCounter()
```js
let counter = btn.getPressCounter();
```
Returns `0` if the button is not long-pressed. Otherwise, it returns `1` or the counter since the button has been long-pressed. The counter is increased every `pressRepeatTicks` milliseconds if value greater than `0` was provided (see [configuration properties](https://github.com/zendiy-mgos/zbutton#js_zbutton_cfg)).
## Additional resources
Take a look to some other samples or libraries.

|Reference|Type||
|--|--|--|
|[zbutton-gpio](https://github.com/zendiy-mgos/zbutton-gpio)|Library|Mongoose-OS library for attaching ZenButtons to gpio-based pushbuttons.|
|[zbutton-mqtt](https://github.com/zendiy-mgos/zbutton-mqtt)|Library|Mongoose-OS library for publishing ZenButtons events as MQTT messages.|
|[zbutton-mqtt-demo](https://github.com/zendiy-mgos/zbutton-mqtt-demo)|Firmware|Mongoose-OS demo firmware that uses ZenButtons ecosystem for publishing pushbutton events as MQTT messages.|