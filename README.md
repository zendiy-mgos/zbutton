# ZenButton
## Overview
Mongoose-OS library for ZenButtons ecosystem.

## GET STARTED
Build up your own device in few minutes just starting from one of the following samples.

|Sample|Notes|
|--|--|
|[zbutton-mqtt-demo](https://github.com/zendiy-mgos/zbutton-mqtt-demo)|Mongoose-OS demo firmware that uses ZenButtons ecosystem for publishing pushbutton events as MQTT messages.|
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
  MGOS_EV_ZBUTTON_ON_ANY,
  MGOS_EV_ZBUTTON_ON_DOWN,
  MGOS_EV_ZBUTTON_ON_UP,
  MGOS_EV_ZBUTTON_ON_CLICK,
  MGOS_EV_ZBUTTON_ON_DBLCLICK,
  MGOS_EV_ZBUTTON_ON_PRESS,
  MGOS_EV_ZBUTTON_ON_PRESS_END
};
```
Button events. They can be classified in 2 different categories:
- **Listening**: the button instance is litening to these events, and you can raise them using `mgos_event_trigger()` for driving the instance (e.g.: the [zbutton-gpio](https://github.com/zendiy-mgos/zbutton-gpio) library uses these events for implementing gpio-based buttons ).
- **Publishing**: the button instance publishes these events, so you can subcribe to them using `mgos_event_add_handler()` or `mgos_event_add_group_handler()`.

|Event|Type||
|--|--|--|
|MGOS_EV_ZBUTTON_ON_ANY|PUBLISHING|Subscribe to this event using `mgos_event_add_group_handler()`.|
|MGOS_EV_ZBUTTON_ON_DOWN|LISTENING|Send this event to the button instance when the phisical button of your device is pushed down.|
|MGOS_EV_ZBUTTON_ON_UP|LISTENING|Send this event to the button instance when the phisical button of your device is released.|
|MGOS_EV_ZBUTTON_ON_CLICK|PUBLISHING|Published when the button is clicked (single-click).|
|MGOS_EV_ZBUTTON_ON_DBLCLICK|PUBLISHING|Published when the button is double-clicked.|
|MGOS_EV_ZBUTTON_ON_PRESS|PUBLISHING|Published when the button is pressed (long-press).|
|MGOS_EV_ZBUTTON_ON_PRESS_END|PUBLISHING|Published when the button press (long-press) ends.|

**Example 1** - A pushbutton on pin 14 sends its state to the button instance.
```c
void mg_zbutton_gpio_button_handler_cb(int pin, void *arg) {
  struct mgos_zbutton *handle = (struct mgos_zbutton *)arg;
  bool gpio_val = mgos_gpio_read(pin);  
  mgos_event_trigger(gpio_val ? MGOS_EV_ZBUTTON_ON_DOWN : MGOS_EV_ZBUTTON_ON_UP, handle);
  LOG(LL_DEBUG, ("Triggering button %s on pin %d ('%s').", gpio_val ? "DOWN" : "UP", pin, handle->id));
}

struct mgos_zbutton *btn = mgos_zbutton_create("btn-1", NULL);
// Set the pushbutton handler
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
    } else if (ev == MGOS_EV_ZBUTTON_ON_PRESS_END) {
      LOG(LL_INFO, ("Button '%s' RELEASED (pressed for %dms)", handle->id,
        mgos_zbutton_press_duration_get(handle)));
    }
  }
  (void) ud;
}
mgos_event_add_group_handler(MGOS_EV_ZBUTTON_ON_ANY, mg_btn_on_event_cb, NULL);
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
|press_ticks|Press duration, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS ` to use the default duration (1s).|
|press_repeat_ticks|Interval in milliseconds, for raising multiple `MGOS_EV_ZBUTTON_ON_PRESS` events, subsequent to the first one. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_PRESS_TICKS` to use the default interval (1s). Set to `0` to disable event repetition.|
|debounce_ticks|Debouncing time, in milliseconds. Set to `-1` or to `MGOS_ZBUTTON_DEFAULT_DEBOUNCE_TIMEOUT` to use the default timeout (50ms). Set to `0` to disable it.|

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
// press_ticks          => equals to MGOS_ZBUTTON_DEFAULT_PRESS_TICKS
// press_repeat_ticks   => equals to MGOS_ZBUTTON_DEFAULT_PRESS_TICKS
// debounce_ticks       => equals to MGOS_ZBUTTON_DEFAULT_DEBOUNCE_TICKS

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
Returns how long the button has been pressed (in milliseconds). Returns `0` if the button is not pressed (long-press).

|Parameter||
|--|--|
|handle|Button handle.|
### mgos_zbutton_press_counter_get()
```c
int mgos_zbutton_press_counter_get(struct mgos_zbutton *handle);
```
Returns `0` if the button is not pressed. Otherwise, it returns `1` or the counter since the button has been pressed (long-press). The counter is increased every `press_repeat_ticks` milliseconds if value greater than 0(zero) was provided.

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
Button events. They can be classified in 2 different categories:
- **Listening**: the button instance is litening to these events, and you can raise them using `Event.trigger()` for driving the instance (e.g.: the [zbutton-gpio](https://github.com/zendiy-mgos/zbutton-gpio) library uses these events for implementing gpio-based buttons ).
- **Publishing**: the button instance publishes these events, so you can subcribe to them using `Event.addHandler()` or `Event.addGroupHandler()`.

|Event|Type||
|--|--|--|
|EV_ON_ANY|PUBLISHING|Subscribe to this event using `Event.addGroupHandler()`.|
|EV_ON_DOWN|LISTENING|Send this event to the button instance when the phisical button of your device is pushed down.|
|EV_ON_UP|LISTENING|Send this event to the button instance when the phisical button of your device is released.|
|EV_ON_CLICK|PUBLISHING|Published when the button is clicked (single-click).|
|EV_ON_DBLCLICK|PUBLISHING|Published when the button is double-clicked.|
|EV_ON_PRESS|PUBLISHING|Published when the button is pressed (long-press).|
|EV_ON_PRESS_END|PUBLISHING|Published when the button press (long-press) ends.|

**Example 1** - A pushbutton on pin 14 sends its state to the button instance.
```js
let btn1 = ZenButton.create('btn-1');
// Set the pushbutton handler
GPIO.set_button_handler(14, GPIO.PULL_UP, GPIO.INT_EDGE_ANY, 50,
function(pin, btn) {
  let gpioVal = GPIO.read(pin);
  let handle = ZenThing.getHandle(btn);
  Event.trigger((gpioVal ? ZenButton.EV_ZBUTTON_DOWN : ZenButton.EV_ZBUTTON_UP), handle);
  print("Triggering", btn.id, "button", (gpioVal ? "DOWN" : "UP"), "on pin", pin);
}, btn1);
```
**Example 2** - Subscribe to the button events.
```js
function onBtnEvent(ev, evdata, ud) {
  let btn = ZenThing.getFromHandle(evdata);
  if (ev === ZenButton.EV_ON_CLICK) {
    print("Button", btn.id, "CLICKED");
  } else if (ev === ZenButton.EV_ON_DBLCLICK) {
    print("Button", btn.id, "DOUBLE-CLICKED");
  } else if (ev === ZenButton.EV_ON_PRESS) {
    print("Button", btn.id, "PRESSED", btn.getPressCounter());
  } else if (ev === ZenButton.EV_ON_PRESS_END) {
    print("Button", btn.id, "RELESED after", btn.getPressDuration());
  }
}
Event.addGroupHandler(ZenButton.EV_ON_ANY, onBtnEvent, null);
```
### ZenButton.create()
```js
let btn = ZenButton.create(id, cfg);
```
Creates and initializes the switch instance. Returns the instance, or `null` on error.

|Parameter|Type||
|--|--|--|
|id|string|Unique ZenThing ID.|
|cfg|object|Optional. Button configuration. If missing, default configuration values are used. For more details see *'Button configuration properties'* below.|

**Button configuration properties**
```js
{
  clickTicks: 600,          //ms
  pressTicks: 1000,         //1s
  pressRepeatTicks: 1000,   //1s
  debounceTicks: 50         //ms
}
```
|Property|Type||
|--|--|--|
|clickTicks|numeric|Optional. Single click duration, in milliseconds. Default value 600ms.|
|pressTicks|numeric|Optional. Press duration, in milliseconds. Default value 1s.|
|pressRepeatTicks|numeric|Optional. Interval in milliseconds, for raising multiple `ZenButton.EV_ON_PRESS` events, subsequent to the first one. Set to `0` to disable event repetition. Default value 1s.|
|debounceTicks|numeric|Optional. Debouncing time, in milliseconds, Set to `0` to disable it. Default value 50ms.|

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
Returns how long the button has been pressed (in milliseconds). Returns `0` if the button is not pressed (long-press).
### .getPressCounter()
```js
let counter = btn.getPressCounter();
```
Returns `0` if the button is not pressed. Otherwise, it returns `1` or the counter since the button has been pressed (long-press). The counter is increased every `press_repeat_ticks` milliseconds if value greater than 0(zero) was provided.
## Additional resources
Take a look to some other samples or libraries.

|Reference|Type||
|--|--|--|
|[zbutton-gpio](https://github.com/zendiy-mgos/zbutton-gpio)|Library|Mongoose-OS library for attaching ZenButtons to gpio-based pushbuttons.|
|[zbutton-mqtt](https://github.com/zendiy-mgos/zbutton-mqtt)|Library|Mongoose-OS library for publishing ZenButtons events as MQTT messages.|
