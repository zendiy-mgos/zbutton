load('api_zthing.js');

let ZenButton = {
  _crt: ffi('void *mgos_zbutton_create(char *, void *)'),
  _ispr: ffi('bool mgos_zbutton_is_pressed(void *)'),
  _pdget: ffi('int mgos_zbutton_press_duration_get(void *)'),
  _pcget: ffi('int mgos_zbutton_press_counter_get(void *)'),
  _rst: ffi('void mgos_zbutton_reset(void *)'), 
  _cls: ffi('void mgos_zbutton_close(void *)'),
  _cfgc: ffi('void *mjs_zbutton_cfg_create(int, int, int, int, int)'),

  THING_TYPE: (8 | ZenThing.TYPE_SENSOR),

  // Event codes.
  EV_ZBUTTON_DOWN: 1514294784, // Send this event to the button instance when the phisical button of your devide is pushed down.
  EV_ZBUTTON_UP: 1514294785,   // Send this event to the button instance when the phisical button of your devide is released.
  EV_ON_CLICK: 1514294786,     // Published when the button is clicked (single-click).
  EV_ON_DBLCLICK: 1514294787,  // Published when the button is bouble-clicked.
  EV_ON_PRESS: 1514294788,     // Published when the button is pressed (long-press).

  // ## **`ZenButton.create(id, cfg)`**
  //
  // Example:
  // ```javascript
  // let sw = ZenButton.create('btn1');
  // ```
  create: function(id, cfg) {
    let cfgo = null;
    if (cfg) {
      cfgo = ZenButton._cfgc(
        ZenThing._getSafe(clickTicks, -1),
        ZenThing._getSafe(dblclickDelayTicks, -1),
        ZenThing._getSafe(pressTicks, -1),
        ZenThing._getSafe(pressRepeatTicks, -1),
        ZenThing._getSafe(pressTimeout, -1)
      );
      if (cfgo == null) return null;
    }
    // create the handle
    let handle = ZenButton._crt(id, cfgo);
    ZenThing._free(cfgo);
    if (!handle) return null; 
    
    // create the JS instance
    let obj = Object.create(ZenButton._proto);
    obj.handle = handle;
    obj.id = id;
    obj.type = ZenButton.THING_TYPE;
    
    // raise on-create event
    ZenThing._onCreatePub(obj);

    return obj;
  },

  _proto: {
    handle: null,
    id: null,
    
    isPressed: function() {
      return ZenButton._ispr(this.handle); 
    },

    getPressDuration: function() {
      return ZenButton._pdget(this.handle); 
    },

    getPressCounter: function() {
      return ZenButton._pcget(this.handle); 
    },
    
    reset: function() {
      ZenButton._rst(this.handle);
    },
    
    close: function() {
      ZenButton._cls(this.handle);
    },
  },
};