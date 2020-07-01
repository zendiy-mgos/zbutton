#include "mgos.h"
#include "mgos_timers.h"
#include "mgos_zbutton.h"

#ifdef MGOS_HAVE_MJS
#include "mjs.h"
#endif /* MGOS_HAVE_MJS */

enum mg_zbutton_state {
  ZBUTTON_STATE_IDLE,
  ZBUTTON_STATE_FIRST_DOWN,
  ZBUTTON_STATE_FIRST_UP,
  ZBUTTON_STATE_SECOND_DOWN,
  ZBUTTON_STATE_SECOND_UP,
  ZBUTTON_STATE_PRESS
};

struct mg_zbutton {
  MGOS_ZBUTTON_BASE
  struct mgos_zbutton_cfg cfg;
  enum mg_zbutton_state state;
  int64_t last_btndown_time;
  int64_t last_btnup_time;
  int press_counter;
  int press_timer_id;
};

#define MG_ZBUTTON_CAST(h) ((struct mg_zbutton *)h)

bool mg_zbutton_cfg_set(struct mgos_zbutton *handle,
                        struct mgos_zbutton_cfg *cfg) {
  if (!handle) return false;
  if (cfg != NULL) {
    if (cfg->click_ticks <= 0) {
      LOG(LL_ERROR, ("Invalid config: 'click_ticks' must be greater than 0(zero)"));
      return false;
    }
    if (cfg->dblclick_delay_ticks <= 0) {
      LOG(LL_ERROR, ("Invalid config: 'dblclick_delay_ticks' must be greater than 0(zero)"));
      return false;
    }
    if (cfg->press_ticks > 0 && cfg->press_ticks <= cfg->click_ticks) {
      LOG(LL_ERROR, ("Invalid config: 'press_ticks' must be greater than 'click_ticks' (>%d).", cfg->click_ticks));
      return false;
    }
    if (cfg->press_timeout > 0 && cfg->press_timeout <= cfg->press_ticks) {
      LOG(LL_ERROR, ("Invalid config: 'press_timeout' must be greater than 'press_ticks' (>%d).", cfg->press_ticks));
      return false;
    }
  }

  struct mg_zbutton *h = MG_ZBUTTON_CAST(handle);
  h->cfg.click_ticks = (cfg == NULL ? MGOS_ZBUTTON_DEFAULT_CLICK_TICKS : cfg->click_ticks);
  h->cfg.dblclick_delay_ticks = (cfg == NULL ? MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS : cfg->dblclick_delay_ticks);
  h->cfg.press_ticks = (cfg == NULL ? MGOS_ZBUTTON_DEFAULT_PRESS_TICKS : cfg->press_ticks);
  h->cfg.press_repeat_ticks = (cfg == NULL ? MGOS_ZBUTTON_DEFAULT_PRESS_TICKS : cfg->press_repeat_ticks);  
  h->cfg.press_timeout = (cfg == NULL ? MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT : cfg->press_timeout);
  return true;
}

struct mgos_zbutton *mgos_zbutton_create(const char *id,
                                         struct mgos_zbutton_cfg *cfg) {
  if (id == NULL) return NULL;
  struct mg_zbutton *handle = calloc(1, sizeof(struct mg_zbutton));
  if (handle != NULL) {
    handle->id = strdup(id);
    handle->type = MGOS_ZTHING_BUTTON;

    if (mg_zbutton_cfg_set(MGOS_ZBUTTON_CAST(handle), cfg)) {
      handle->press_timer_id = MGOS_INVALID_TIMER_ID;
      
      mgos_zbutton_reset(MGOS_ZBUTTON_CAST(handle));

      if (mgos_zthing_register(MGOS_ZTHING_CAST(handle))) {
        /* Trigger the CREATED event */
        mgos_event_trigger(MGOS_EV_ZTHING_CREATED, handle);
        LOG(LL_INFO, ("Button '%s' successfully created.", handle->id));
        return MGOS_ZBUTTON_CAST(handle);
      } else {
        LOG(LL_ERROR, ("Error creating '%s'. Handle registration failed.", id));
      }
    }
    free(handle->id);
    free(handle);
  } else {
    LOG(LL_ERROR, ("Error creating '%s'. Memory allocation failed.", id));
  }
  return NULL;
}

void mg_zbutton_press_timer_clear(struct mg_zbutton *handle) {
  if (handle && (handle->press_timer_id != MGOS_INVALID_TIMER_ID)) {
    mgos_clear_timer(handle->press_timer_id);
    handle->press_timer_id = MGOS_INVALID_TIMER_ID;
    LOG(LL_DEBUG, ("Long-press timer sucessfully stopped ('%s')", handle->id));
  }
}

int mgos_zbutton_press_duration_get(struct mgos_zbutton *handle) {
  if (!handle) return -1;
  struct mg_zbutton *h = MG_ZBUTTON_CAST(handle);
  return (h->state == ZBUTTON_STATE_PRESS ? ((mgos_uptime_micros() - h->last_btndown_time) / 1000) : -1);
}

int mgos_zbutton_press_counter_get(struct mgos_zbutton *handle) {
  if (!handle) return -1;
  struct mg_zbutton *h = MG_ZBUTTON_CAST(handle);
  return (h->state == ZBUTTON_STATE_PRESS ? h->press_counter : -1);
}

bool mgos_zbutton_is_pressed(struct mgos_zbutton *handle) {
  if (!handle) return false;
  return (MG_ZBUTTON_CAST(handle)->state == ZBUTTON_STATE_PRESS);
}

void mgos_zbutton_reset(struct mgos_zbutton *handle) {
  struct mg_zbutton *h = MG_ZBUTTON_CAST(handle);
  mg_zbutton_press_timer_clear(h);
  if (h) {
    h->state = ZBUTTON_STATE_IDLE;
    h->press_counter = 0;
    h->last_btndown_time = 0;
    h->last_btnup_time = 0;
    LOG(LL_DEBUG, ("Resetting to IDLE state ('%s').", h->id));
  }
}

void mgos_zbutton_close(struct mgos_zbutton *handle) {
  struct mg_zbutton *h = MG_ZBUTTON_CAST(handle);
  mg_zbutton_press_timer_clear(h);
  mgos_zthing_close(MGOS_ZTHING_CAST(h));
}

static void mg_zbutton_press_timer_cb(void *arg) {
  if (!arg) return;
  
  struct mg_zbutton *h = (struct mg_zbutton *)arg;
  if (h->state == ZBUTTON_STATE_FIRST_DOWN) {
    LOG(LL_DEBUG, ("Running into the long-press timer ('%s')", h->id));
    // Start the press-repeat timer
    if (h->cfg.press_repeat_ticks > 0) {
      h->press_timer_id = mgos_set_timer(h->cfg.press_ticks, MGOS_TIMER_REPEAT, mg_zbutton_press_timer_cb, h);
      if (h->press_timer_id == MGOS_INVALID_TIMER_ID) {
        LOG(LL_ERROR, ("Unable to start long-press repeat timer. The long-press repeat feature won't work properly."));
      }
    } else {
      h->press_timer_id = MGOS_INVALID_TIMER_ID;
    }
    h->state = ZBUTTON_STATE_PRESS;
  }

  bool timeout = (h->last_btndown_time > 0 ? (((mgos_uptime_micros() - h->last_btndown_time) / 1000) >= h->cfg.press_timeout) : false);
  if ((h->state != ZBUTTON_STATE_PRESS) || timeout) {
    mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h));
    return;
  }

  ++h->press_counter;
  LOG(LL_DEBUG, ("Triggering ON_PRESS event #%d ('%s')", h->press_counter, h->id));
  mgos_event_trigger(MGOS_EV_ZBUTTON_ON_PRESS, h);
}

void mg_zbutton_on_btndown(struct mg_zbutton *h) {
  LOG(LL_DEBUG, ("Triggered BTN DOWN ('%s')", h->id));
  int64_t now = mgos_uptime_micros();
  int ticks = ((now - h->last_btnup_time) / 1000);

  // if (h->state == ZBUTTON_STATE_FIRST_UP && ticks > h->cfg.dblclick_delay_ticks) {
  //   LOG(LL_DEBUG, ("Zombie waiting for double click detectd. Resetting to IDLE state ('%s').", h->id));
  //   mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h));  
  // }

  if (h->state == ZBUTTON_STATE_IDLE) {
    // Firts click starts...
    LOG(LL_DEBUG, ("Entering in FIRST_DOWN state ('%s')", h->id));
    h->state = ZBUTTON_STATE_FIRST_DOWN;
    h->last_btnup_time = 0;
    h->last_btndown_time = now;
    if (h->cfg.press_ticks > 0) {
      h->press_timer_id = mgos_set_timer(h->cfg.press_ticks, 0, mg_zbutton_press_timer_cb, h);
      if (h->press_timer_id == MGOS_INVALID_TIMER_ID) {
        LOG(LL_ERROR, ("Unable to start long-press timer. The long-press feature won't work properly ('%s').", h->id));
      } else {
        LOG(LL_DEBUG, ("Long-press timer successfully started ('%s').", h->id));
      }
    }
    return;
  } else if (h->state == ZBUTTON_STATE_FIRST_UP) {  
    if (ticks <= h->cfg.dblclick_delay_ticks) {
      // Second click starts...
      LOG(LL_DEBUG, ("Entering in SECOND_DOWN state ('%s')", h->id));
      h->state = ZBUTTON_STATE_SECOND_DOWN;
      h->last_btndown_time = now;
      return;
    }
  } else {
    // Unexpected state/s, so I reset to ZBUTTON_STATE_IDLE
    //   - ZBUTTON_STATE_SECOND_UP
    //   - ZBUTTON_STATE_FIRST_DOWN
    //   - ZBUTTON_STATE_SECOND_DOWN
    //   - ZBUTTON_STATE_PRESS
    LOG(LL_DEBUG, ("Unexpected state %d ('%s').", h->state, h->id));
  }

  mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h));
}

void mg_zbutton_on_btnup(struct mg_zbutton *h) {
  LOG(LL_DEBUG, ("Triggered BTN UP on '%s'.", h->id));
  
  if (h->state == ZBUTTON_STATE_IDLE) {
    // Unexpected state, but nothing to do with.
    LOG(LL_DEBUG, ("Unexpected ILDE state ('%s')", h->id));
    return;
  } else if (h->state == ZBUTTON_STATE_FIRST_DOWN || h->state == ZBUTTON_STATE_SECOND_DOWN) {
    int64_t now = mgos_uptime_micros();
    int ticks = ((now - h->last_btndown_time) / 1000);  
    if (h->state == ZBUTTON_STATE_SECOND_DOWN) {
      // This is the second click (second up), so I trigger the double-click event
      // and then reset to ZBUTTON_STATE_IDLE.
      LOG(LL_DEBUG, ("Entering in SECOND_UP state ('%s')", h->id));
      h->state = ZBUTTON_STATE_SECOND_UP;
      h->last_btnup_time = now;
      LOG(LL_DEBUG, ("Triggering ON_DBLCLICK event ('%s')", h->id));
      mgos_event_trigger(MGOS_EV_ZBUTTON_ON_DBLCLICK, h);
    } else {
      // h->state equals to ZBUTTON_STATE_FIRST_DOWN
      LOG(LL_DEBUG, ("Entering in FIRST_UP state ('%s')", h->id));
      h->last_btnup_time = now;
      h->state = ZBUTTON_STATE_FIRST_UP;
      
      // This maybe the first click of a double-click
      if (ticks < h->cfg.click_ticks) {
        LOG(LL_DEBUG, ("Waiting for a second click ('%s')", h->id));
        return;
      }

      if (h->cfg.press_ticks <= 0 || ticks < h->cfg.press_ticks) {
        // This is a single click, so I trigger the click event
        // and then reset to ZBUTTON_STATE_IDLE.
        LOG(LL_DEBUG, ("Triggering ON_CLICK event ('%s')", h->id));
        mgos_event_trigger(MGOS_EV_ZBUTTON_ON_CLICK, h);
      } else {
        LOG(LL_DEBUG, ("The button push seems to be a %dms press ('%s')", ticks, h->id));
      }
    }
  } else if (h->state == ZBUTTON_STATE_FIRST_UP || h->state == ZBUTTON_STATE_SECOND_UP) {
    // Unexpected state, so I reset to ZBUTTON_STATE_IDLE.
    LOG(LL_DEBUG, ("Unexpected state %d ('%s').", h->state, h->id));
  } else if (h->state == ZBUTTON_STATE_PRESS) {
    // Long press ended, so I reset to ZBUTTON_STATE_IDLE.
    LOG(LL_DEBUG, ("End of ON_PRESS event ('%s').", h->id));
  }

  mgos_zbutton_reset(MGOS_ZBUTTON_CAST(h));
}

void mg_zbutton_btndown_btnup_cb(int ev, void *ev_data, void *userdata) {
  if (ev_data != NULL) {
    struct mg_zbutton *h = MG_ZBUTTON_CAST(ev_data);
    if (ev == MGOS_EV_ZBUTTON_DOWN) {
      mg_zbutton_on_btndown(h);
    } else if (ev == MGOS_EV_ZBUTTON_UP) {
      mg_zbutton_on_btnup(h);
    }
  }
  (void) userdata;
}

#ifdef MGOS_HAVE_MJS

struct mgos_zbutton_cfg *mjs_zbutton_cfg_create(int click_ticks,
                                                int dblclick_delay_ticks,
                                                int press_ticks,
                                                int press_repeat_ticks,
                                                int press_timeout) {
  struct mgos_zbutton_cfg *cfg = calloc(1, sizeof(struct mgos_zbutton_cfg));
  if (cfg != NULL) {
    cfg->click_ticks = (click_ticks <= 0 ? 
      MGOS_ZBUTTON_DEFAULT_CLICK_TICKS : click_ticks);
    cfg->dblclick_delay_ticks = (dblclick_delay_ticks <= 0 ? 
      MGOS_ZBUTTON_DEFAULT_DBLCLICK_DELAY_TICKS : dblclick_delay_ticks);
    cfg->press_ticks = (press_ticks == -1 ? 
      MGOS_ZBUTTON_DEFAULT_PRESS_TICKS : press_ticks);
    cfg->press_repeat_ticks = (press_repeat_ticks == -1 ? 
      MGOS_ZBUTTON_DEFAULT_PRESS_TICKS : press_repeat_ticks);
    cfg->press_timeout = (press_timeout == -1 ? 
      MGOS_ZBUTTON_DEFAULT_PRESS_TIMEOUT : press_timeout);
  }
  return cfg;
}

#endif /* MGOS_HAVE_MJS */


bool mgos_zbutton_init() {
  if (!mgos_event_register_base(MGOS_ZBUTTON_EVENT_BASE, "ZenButton events")) {
    return false;
  }
  if (!mgos_event_add_handler(MGOS_EV_ZBUTTON_DOWN, mg_zbutton_btndown_btnup_cb, NULL) ||
      !mgos_event_add_handler(MGOS_EV_ZBUTTON_UP, mg_zbutton_btndown_btnup_cb, NULL)) {
    return false;
  }

  //LOG(LL_INFO, ("MGOS_ZBUTTON_EVENT_BASE %d", MGOS_ZBUTTON_EVENT_BASE));

  return true;
}