# ZenButton
## Overview
A Mongoose OS library for ZenButtons ecosystem.

## GET STARTED
Build up your own device in few minutes just starting from one of the following samples.

|Sample|Notes|
|--|--|
|[zbutton-mqtt-demo](https://github.com/zendiy-mgos/zbutton-mqtt-demo)|Mongoose OS demo firmware for using ZenButtons and MQTT.|
## Usage
Include the library into your `mos.yml` file.
```yaml
libs:
  - origin: https://github.com/zendiy-mgos/zbutton
```
If you are developing a JavaScript firmware, load `api_zbutton.js` in your `init.js` file.
```js
load('api_zbutton.js');
```
## C/C++ API Reference
### mgos_zbutton_event
```c
enum mgos_zbutton_event {
  MGOS_EV_ZBUTTON_DOWN = MGOS_ZBUTTON_EVENT_BASE,
  MGOS_EV_ZBUTTON_UP,
  MGOS_EV_ZBUTTON_ON_CLICK,
  MGOS_EV_ZBUTTON_ON_DBLCLICK,
  MGOS_EV_ZBUTTON_ON_PRESS
};
```
Button events. They can be classified in 2 different categories:
- **Listening**: the button instance is litening to these events, and you can raise them using `mgos_event_trigger()` for driving the instance (e.g.: the [zbutton-gpio](https://github.com/zendiy-mgos/zbutton-gpio) library uses these events for implementing gpio-based buttons ).
- **Publishing**: the button instance publishes these events, so you can subcribe to them using `mgos_event_add_handler()`.

|Event|Type||
|--|--|--|
|MGOS_EV_ZBUTTON_DOWN|LISTENING|Send this event to the button instance when the phisical button of your devide is pushed down.|
|MGOS_EV_ZBUTTON_UP|LISTENING|Send this event to the button instance when the phisical button of your devide is released.|
|MGOS_EV_ZBUTTON_ON_CLICK|PUBLISHING|Published when the button is clicked (single-click).|
|MGOS_EV_ZBUTTON_ON_DBLCLICK|PUBLISHING|Published when the button is bouble-clicked.|
|MGOS_EV_ZBUTTON_ON_PRESS|PUBLISHING|Published when the button is pressed (long-press).|

**Example 1** - Connect a gpio-based push button to the button instance.
```c
void mg_zbutton_gpio_button_handler_cb(int pin, void *arg) {
  struct mgos_zbutton *handle = (struct mgos_zbutton *)arg;
  bool gpio_val = mgos_gpio_read(pin);  
  mgos_event_trigger(gpio_val ? MGOS_EV_ZBUTTON_DOWN : MGOS_EV_ZBUTTON_UP, handle);
  LOG(LL_DEBUG, ("Triggering button %s on pin %d ('%s').", gpio_val ? "DOWN" : "UP", pin, handle->id));
}

struct mgos_zbutton *btn = mgos_zbutton_create("btn-1", NULL);
mgos_gpio_set_button_handler(14, MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_ANY,
  50, mg_zbutton_gpio_button_handler_cb, btn);
```
**Example 2** - Subscribe to the button events.
```c
void mg_btn_on_event_cb(int ev, void *ev_data, void *ud) {
  struct mgos_zbutton *handle = (struct mgos_zbutton *)ev_data;
  if (handle) {
    if (ev == MGOS_EV_ZBUTTON_ON_CLICK) {
      LOG(LL_INFO, ("Button '%s' CLICKED", handle->id));
    } else if (ev == MGOS_EV_ZBUTTON_ON_DBLCLICK) {
      LOG(LL_INFO, ("Button '%s' DOUBLE-CLICKED", handle->id));
    } else if (ev == MGOS_EV_ZBUTTON_ON_PRESS) {
      LOG(LL_INFO, ("Button '%s' PRESSED (#%d)", handle->id,
        mgos_zbutton_press_counter_get(handle)));
    }
  }
  (void) ud;
}
mgos_event_add_handler(MGOS_EV_ZBUTTON_ON_CLICK, mg_btn_on_event_cb, NULL);
mgos_event_add_handler(MGOS_EV_ZBUTTON_ON_DBLCLICK, mg_btn_on_event_cb, NULL);
mgos_event_add_handler(MGOS_EV_ZBUTTON_ON_PRESS, mg_btn_on_event_cb, NULL); 
```
### mgos_zbutton
```c
struct mgos_zbutton {
  char *id;
  int type;
};
```
Button handle. You can get a valid handle using `gos_zbutton_create()`.

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
  int dblclick_delay_ticks;
  int press_ticks;
  int press_repeat_ticks;
  int press_timeout;
};
```
Button configuration values (e.g.: used by  `gos_zbutton_create()`).

|Field||
|--|--|
|click_ticks|Single click duration, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_CLICK_TICKS` to use the default 140ms duration.|
|dblclick_delay_ticks|The delay between the two double-click clicks, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS` to use the default delay (160ms).|
|press_ticks|Press duration, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS ` to use the default duration (1.5s).|
|press_repeat_ticks|Interval in milliseconds, for raising multiple `MGOS_EV_ZBUTTON_ON_PRESS` events, subsequent to the first one. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS` to use the default interval (1.5s). Set to `0` to disable event repetition.|
|press_timeout|Maximum time, in milliseconds, the button can stay pressed. When the timeout expires, the button is reset. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT` to use the default timeout (15s). Set to `0` to disable the timeout.|

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
|cfg|Optional. Button configuration. If `NULL`, default configuration values are used.|

**Example 1** - Create a button using default configuration values.
```c
// click_ticks          => equals to MGOS_ZBUTTON_DEFAULT_CLICK_TICKS
// dblclick_delay_ticks => equals to MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS
// press_ticks          => equals to MGOS_ZBUTTON_DEFAULT_PRESS_TICKS
// press_repeat_ticks   => equals to MGOS_ZBUTTON_DEFAULT_PRESS_TICKS
// press_timeout        => equals to MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT

struct mgos_zbutton *btn = mgos_zbutton_create("btn-1", NULL);
```
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
Returns `true` if the button has been pressed (long-press), otherwise `false`.

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_press_duration_get()
```c
int mgos_zbutton_press_duration_get(struct mgos_zbutton *handle);
```
Returns how long the button has been pressed (in milliseconds). Returns `-1` if the button is not still pressed (long-press).

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_press_counter_get()
```c
int mgos_zbutton_press_counter_get(struct mgos_zbutton *handle);
```
Returns the counter since the button has been pressed (long-press). Returns `-1` if the button is not still pressed. The counther is increased every `press_repeat_ticks` milliseconds, if a configuration value greater than 0(zero) was provided.

|Parameter||
|--|--|
|handle|Button handle.|
## JS API Reference
### ZenButton.create()
```js
let btn = ZenButton.create(id, cfg);
```
Creates and initializes the switch instance. Returns the instance, or `null` on error.

|Parameter|Type||
|--|--|--|
|id|string|Unique ZenThing ID.|
|cfg|object|Optional. Button configuration. If missing, default configuration values are used.|

**Button configuration properties**
|Property|Type||
|--|--|--|
|*cfg*.clickTicks|numeric|Optional. Single click duration, in milliseconds.|
|*cfg*.dblclickDelayTicks|numeric|Optional. The delay between the two double-click clicks, in milliseconds.|
|*cfg*.pressTicks|numeric|Optional. Press duration, in milliseconds.|
|*cfg*.pressRepeatTicks|numeric|Optional. Interval in milliseconds, for raising multiple `ZenButton.EV_ON_PRESS` events, subsequent to the first one. Set to `0` to disable event repetition.|
|*cfg*.pressTimeout|numeric|Optional. Maximum time, in milliseconds, the button can stay pressed. When the timeout expires, the button is reset. Set to `0` to disable the timeout.|

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
Returns `true` if the button has been pressed (long-press), otherwise `false`.
### .getPressDuration()
```js
let duration = btn.getPressDuration(); //milliseconds
```
Returns how long the button has been pressed (in milliseconds). Returns `-1` if the button is not still pressed (long-press).
### .getPressCounter()
```js
let counter = btn.getPressCounter();
```
Returns the counter since the button has been pressed (long-press). Returns `-1` if the button is not still pressed. The counther is increased every `pressRepeatTicks` milliseconds, if a configuration value greater than 0(zero) was provided.
## Additional resources
No additional resources available.