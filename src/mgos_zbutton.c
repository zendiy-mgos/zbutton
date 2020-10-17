#include "mgos.h"
#include "mgos_timers.h"
#include "mgos_zbutton.h"

#ifdef MGOS_HAVE_MJS
#include "mjs.h"
#endif /* MGOS_HAVE_MJS */

enum mg_zbutton_state {
  ZBUTTON_STATE_UP,
  ZBUTTON_STATE_DOWN,
  ZBUTTON_STATE_FIRST_UP,
  ZBUTTON_STATE_SECOND_DOWN,
  ZBUTTON_STATE_PRESSED
};

struct mg_zbutton {
  MGOS_ZBUTTON_BASE
  struct mgos_zbutton_cfg cfg;
  enum mg_zbutton_state state;
  enum mg_zbutton_state state_req;
  int64_t start_time;
  int64_t stop_time;
  int press_counter;
  int tick_timer_id;
};

#define MG_ZBUTTON_CAST(h) ((struct mg_zbutton *)h)

bool mg_zbutton_cfg_set(struct mgos_zbutton_cfg *cfg_src,
                        struct mgos_zbutton_cfg *cfg_dest) {
  if (!cfg_dest) return false;
  if (cfg_src != NULL) {
    if (cfg_src->click_ticks == 0) {
      LOG(LL_ERROR, ("Invalid config: 'click_ticks' must be greater than 0(zero)"));
      return false;
    }
    if (cfg_src->press_ticks == 0 || cfg_src->press_ticks < cfg_src->click_ticks) {
      LOG(LL_ERROR, ("Invalid config: 'press_ticks' must be greater than %d", cfg_src->click_ticks));
      return false;
    }

    cfg_dest->click_ticks = (cfg_src->click_ticks == -1 ? MGOS_ZBUTTON_DEFAULT_CLICK_TICKS : cfg_src->click_ticks);
    cfg_dest->press_ticks = (cfg_src->press_ticks == -1 ? MGOS_ZBUTTON_DEFAULT_PRESS_TICKS : cfg_src->press_ticks);
    cfg_dest->press_repeat_ticks = (cfg_src->press_repeat_ticks == -1 ? MGOS_ZBUTTON_DEFAULT_PRESS_TICKS : cfg_src->press_repeat_ticks);
    cfg_dest->debounce_ticks = (cfg_src->debounce_ticks == -1 ? MGOS_ZBUTTON_DEFAULT_DEBOUNCE_TICKS : cfg_src->debounce_ticks);
  } else {
    cfg_dest->click_ticks = MGOS_ZBUTTON_DEFAULT_CLICK_TICKS;
    cfg_dest->press_ticks = MGOS_ZBUTTON_DEFAULT_PRESS_TICKS;
    cfg_dest->press_repeat_ticks = MGOS_ZBUTTON_DEFAULT_PRESS_TICKS;
    cfg_dest->debounce_ticks = MGOS_ZBUTTON_DEFAULT_DEBOUNCE_TICKS;
  }
  return true;
}

static void mg_zbutton_tick_cb(void *arg);
struct mgos_zbutton *mgos_zbutton_create(const char *id,
                                         struct mgos_zbutton_cfg *cfg) {
  if (id == NULL) return NULL;
  struct mg_zbutton *handle = calloc(1, sizeof(struct mg_zbutton));
  if (handle != NULL) {
    handle->id = strdup(id);
    handle->type = MGOS_ZTHING_BUTTON;

    if (mg_zbutton_cfg_set(cfg, &handle->cfg)) {
      mgos_zbutton_reset(MGOS_ZBUTTON_CAST(handle));

      handle->tick_timer_id = mgos_set_timer(10, MGOS_TIMER_REPEAT, mg_zbutton_tick_cb, handle);        
      if (handle->tick_timer_id != MGOS_INVALID_TIMER_ID) {
        if (mgos_zthing_register(MGOS_ZTHING_CAST(handle))) {
          /* Trigger the CREATED event */
          mgos_event_trigger(MGOS_EV_ZTHING_CREATED, handle);
          LOG(LL_INFO, ("Button '%s' successfully created.", handle->id));
          return MGOS_ZBUTTON_CAST(handle);
        }
        LOG(LL_ERROR, ("Error creating '%s'. Handle registration failed.", id));
      }
      LOG(LL_ERROR, ("Error creating '%s'. Tick-timer initialization failed.", id));
      mgos_clear_timer(handle->tick_timer_id);
    }
    free(handle->id);
    free(handle);
  } else {
    LOG(LL_ERROR, ("Error creating '%s'. Memory allocation failed.", id));
  }
  return NULL;
}

int mgos_zbutton_press_duration_get(struct mgos_zbutton *handle) {
  return (!handle ? -1 : ((MG_ZBUTTON_CAST(handle)->stop_time - MG_ZBUTTON_CAST(handle)->start_time) / 1000));
}

int mgos_zbutton_press_counter_get(struct mgos_zbutton *handle) {
  return (!handle ? -1 : MG_ZBUTTON_CAST(handle)->press_counter);
}

bool mgos_zbutton_is_pressed(struct mgos_zbutton *handle) {
  return (!handle ? false : (MG_ZBUTTON_CAST(handle)->state == ZBUTTON_STATE_PRESSED));
}

void mgos_zbutton_reset(struct mgos_zbutton *handle) {
  struct mg_zbutton *h = MG_ZBUTTON_CAST(handle);
  if (h) {
    h->state = ZBUTTON_STATE_UP;
    h->state_req = ZBUTTON_STATE_UP;
    h->start_time = 0;
    h->stop_time = 0;
    h->press_counter = 0;
    LOG(LL_DEBUG, ("Resetting to UP state ('%s').", h->id));
  }
}

void mgos_zbutton_close(struct mgos_zbutton *handle) {
  struct mg_zbutton *h = MG_ZBUTTON_CAST(handle);
  if (h) {
    if (h->tick_timer_id != MGOS_INVALID_TIMER_ID) {
      mgos_clear_timer(h->tick_timer_id);
      h->tick_timer_id = MGOS_INVALID_TIMER_ID;
    }
  } 
  mgos_zthing_close(MGOS_ZTHING_CAST(h));
}

static void mg_zbutton_tick_cb(void *arg) {
  if (!arg) return;
  
  struct mg_zbutton *h = (struct mg_zbutton *)arg;
  int64_t now = mgos_uptime_micros();

  // Implementation of the state machine
 
  if (h->state == ZBUTTON_STATE_UP) { // waiting for button being pressed.
    if (h->state_req == ZBUTTON_STATE_DOWN) {
      h->state = ZBUTTON_STATE_DOWN;
      h->start_time = now;
    }

  } else if (h->state == ZBUTTON_STATE_DOWN) { // waiting for button being released.
    int start_ticks = ((now - h->start_time) / 1000);
    if (h->state_req == ZBUTTON_STATE_UP) {
      if (h->cfg.debounce_ticks > 0 && start_ticks < h->cfg.debounce_ticks) {
        // The button was released to quickly so I assume some debouncing.
        // So, go back to STATE_UP without calling a function.
        mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h)); // restart
      } else {
        // The button was released for the firt time.
        h->state = ZBUTTON_STATE_FIRST_UP;
        h->stop_time = now;

        LOG(LL_DEBUG, ("Triggering ON_UP event ('%s')", h->id));
        mgos_event_trigger(MGOS_EV_ZBUTTON_ON_UP, h);
      }
    } else if (h->state_req == ZBUTTON_STATE_DOWN) {
      if (start_ticks > h->cfg.press_ticks) {
        // The button starts being pressed (long press). 
        h->state = ZBUTTON_STATE_PRESSED;
        h->stop_time = now;
        h->press_counter = 1;

        LOG(LL_DEBUG, ("Triggering ON_PRESS event ('%s')", h->id));
        mgos_event_trigger(MGOS_EV_ZBUTTON_ON_PRESS, h);
      }
    }

  } else if (h->state == ZBUTTON_STATE_FIRST_UP) {
    // waiting for button being pressed the second time or timeout.
    if (((now - h->start_time) / 1000) > h->cfg.click_ticks) {
      LOG(LL_DEBUG, ("Triggering ON_UP event ('%s')", h->id));
      mgos_event_trigger(MGOS_EV_ZBUTTON_ON_UP, h);

      LOG(LL_DEBUG, ("Triggering ON_CLICK event ('%s')", h->id));
      mgos_event_trigger(MGOS_EV_ZBUTTON_ON_CLICK, h);

      mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h)); // restart
    } else if (h->state_req == ZBUTTON_STATE_DOWN) {
      if (h->cfg.debounce_ticks == 0 || ((now - h->stop_time) / 1000) > h->cfg.debounce_ticks) {
        h->state = ZBUTTON_STATE_SECOND_DOWN;
        h->start_time = now;
      }
    }

  } else if (h->state == ZBUTTON_STATE_SECOND_DOWN) { // waiting for button being released finally.
    // Stay here for at least h->cfg.debounce_ticks because else we might end up in
    // state 1 if the button bounces for too long.
    if (h->state_req == ZBUTTON_STATE_UP) {
      if (((now - h->start_time) / 1000) > h->cfg.debounce_ticks) {
        // this was a 2 click sequence.
        h->stop_time = now;
        h->state = ZBUTTON_STATE_UP;

        LOG(LL_DEBUG, ("Triggering ON_DBLCLICK event ('%s')", h->id));
        mgos_event_trigger(MGOS_EV_ZBUTTON_ON_DBLCLICK, h);
        
        mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h)); // restart
      }
    }

  } else if (h->state == ZBUTTON_STATE_PRESSED) {

    // The button is pressed (long press).
    // So, waiting for the button being released.
    if (h->state_req == ZBUTTON_STATE_UP) {
      // The button is released after a long press
      h->stop_time = now;
      h->state = ZBUTTON_STATE_UP;
      
      LOG(LL_DEBUG, ("Triggering ON_PRESS_END event ('%s')", h->id));
      mgos_event_trigger(MGOS_EV_ZBUTTON_ON_PRESS_END, h);   
      
      mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h)); // restart
    } else {
      // The button continue being pressed (long press).
      if (h->cfg.press_repeat_ticks > 0 &&
          ((now - h->stop_time) / 1000) >= h->cfg.press_repeat_ticks) {
        h->stop_time = now;
        h->press_counter += 1;

        LOG(LL_DEBUG, ("Triggering ON_PRESS event #%d ('%s')", h->press_counter, h->id));
        mgos_event_trigger(MGOS_EV_ZBUTTON_ON_PRESS, h);
      }
    }
  }
}
         
void mg_zbutton_on_btndown(struct mg_zbutton *h) {
  h->state_req = ZBUTTON_STATE_DOWN;
  LOG(LL_DEBUG, ("Triggered BTN DOWN ('%s')", h->id));
}

void mg_zbutton_on_btnup(struct mg_zbutton *h) {
  h->state_req = ZBUTTON_STATE_UP;
  LOG(LL_DEBUG, ("Triggered BTN UP ('%s')", h->id));
}

void mg_zbutton_events_cb(int ev, void *ev_data, void *userdata) {
  if (ev_data != NULL) {
    struct mg_zbutton *h = MG_ZBUTTON_CAST(ev_data);
    if (ev == MGOS_EV_ZBUTTON_ON_DOWN) {
      mg_zbutton_on_btndown(h);
    } else if (ev == MGOS_EV_ZBUTTON_ON_UP) {
      mg_zbutton_on_btnup(h);
    }
  }
  (void) userdata;
}

#ifdef MGOS_HAVE_MJS

struct mgos_zbutton_cfg *mjs_zbutton_cfg_create(int click_ticks,
                                                int press_ticks,
                                                int press_repeat_ticks,
                                                int debounce_ticks) {
 struct mgos_zbutton_cfg cfg_src = {
    click_ticks,
    press_ticks,
    press_repeat_ticks,
    debounce_ticks
  };
  struct mgos_zbutton_cfg *cfg_dest = calloc(1, sizeof(struct mgos_zbutton_cfg));
  if (mg_zbutton_cfg_set(&cfg_src, cfg_dest)) return cfg_dest;
  free(cfg_dest);
  return NULL;
}

#endif /* MGOS_HAVE_MJS */


bool mgos_zbutton_init() {
  if (!mgos_event_register_base(MGOS_ZBUTTON_EVENT_BASE, "ZenButton events")) {
    return false;
  }
  if (!mgos_event_add_group_handler(MGOS_ZBUTTON_EVENT_BASE, mg_zbutton_events_cb, NULL)) {
    return false;
  }

  //LOG(LL_INFO, ("MGOS_ZBUTTON_EVENT_BASE %d", MGOS_ZBUTTON_EVENT_BASE));

  return true;
}