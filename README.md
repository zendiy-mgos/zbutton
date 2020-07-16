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

**Example** - Use handle fields.
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
Button configuration values for `gos_zbutton_create()`.

|Field||
|--|--|
|click_ticks|Single click duration, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_CLICK_TICKS` to use the default 140ms duration.|
|dblclick_delay_ticks|The delay between the two double-click clicks, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS` to use the default 160ms delay.|
|press_ticks|Press duration, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS ` to use the default 1.5s duration.|
|press_repeat_ticks|Interval in milliseconds, for raising multiple `MGOS_EV_ZBUTTON_ON_PRESS` events, subsequent to the first one. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS ` to use the default 1.5 secs. interval. Set to `0` to disable events.|
|press_timeout|Maximum time, in milliseconds, the button can stay pressed. When the timeout expires, the button is reset. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT ` to use the default 15 secs. timeout. Set to `0` to disable the timeout.|

**Example** - Create and initialize configuration settings.
```c
// create and initialize cfg using defaults
struct mgos_zbutton_cfg cfg = MGOS_ZBUTTON_CFG;
```
### mgos_zbutton_create()
```c
struct mgos_zbutton *mgos_zbutton_create(const char *id, struct mgos_zbutton_cfg *cfg);
```
Create and initialize the button instance. Returns the instance handle, or `NULL` on error.

|Parameter||
|--|--|
|id|Unique ZenThing ID.|
|cfg|Optional. Button configuration. If `NULL`, default configuration values are used.|

**Example 1** - Create a button using default configuration values.
```c
// click_ticks = MGOS_ZBUTTON_DEFAULT_CLICK_TICKS
// dblclick_delay_ticks = MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS
// press_ticks = MGOS_ZBUTTON_DEFAULT_PRESS_TICKS
// press_repeat_ticks = MGOS_ZBUTTON_DEFAULT_PRESS_TICKS
// press_timeout = MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT

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
Returns `true` if the button is pressed (long press), otherwise `false`.

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_press_duration_get()
```c
int mgos_zbutton_press_duration_get(struct mgos_zbutton *handle);
```
Returns, in milliseconds, how long the button is pressed (long press). Returns `-1` if the button is not pressed.

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_press_counter_get()
```c
int mgos_zbutton_press_counter_get(struct mgos_zbutton *handle);
```
Returns the press (long press) counter. Returns `-1` if the button is not pressed. The counther is increased every `press_repeat_ticks` milliseconds if a configuration value greater than 0(zero) was provided.

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
|*cfg*.pressRepeatTicks|numeric|Optional. Interval in milliseconds, for raising multiple `ZenButton.EV_ON_PRESS` events, subsequent to the first one. Set to `0` to disable events.|
|*cfg*.pressTimeout|numeric|Optional. Maximum time, in milliseconds, the button can stay pressed. When the timeout expires, the button is reset. Set to `0` to disable the timeout.|

**Switch instance properties** - The created instance has following properties.
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
Returns `true` if the button is pressed (long press), otherwise `false`.
### .getPressDuration()
```js
let duration = btn.getPressDuration();
```
Returns, in milliseconds, how long the button is pressed (long press). Returns `-1` if the button is not pressed.
### .getPressCounter()
```js
let counter = btn.getPressCounter();
```
Returns the press (long press) counter. Returns `-1` if the button is not pressed. The counther is increased every `pressRepeatTicks` milliseconds if a configuration value greater than 0(zero) was provided.
## Additional resources
No additional resources available.