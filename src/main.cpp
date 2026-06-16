#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "context.h"
#include "rtc_wrapper.h"
#include "screens.h"

struct button_state {
    uint8_t pin;
    bool stable_state;
    bool last_reading;
    bool pressed;
    unsigned long last_change;
};

static struct context ctx;
static enum screen current_screen = INIT_SCR;
static bool screen_dirty = true;

static button_state btn_mode = {BTN1_PIN, HIGH, HIGH, false, 0};
static button_state btn_up = {BTN2_PIN, HIGH, HIGH, false, 0};
static button_state btn_down = {BTN3_PIN, HIGH, HIGH, false, 0};
static button_state btn_select = {BTN4_PIN, HIGH, HIGH, false, 0};

static unsigned long reset_hold_started = 0;
static long last_alarm_key = -1;

static bool is_down(const button_state& button) {
    return button.stable_state == LOW;
}

static void update_button(button_state& button) {
    bool reading = digitalRead(button.pin);
    button.pressed = false;

    if (reading != button.last_reading) {
        button.last_reading = reading;
        button.last_change = millis();
    }

    if (millis() - button.last_change > BUTTON_DEBOUNCE_MS && reading != button.stable_state) {
        button.stable_state = reading;
        button.pressed = (button.stable_state == LOW);
    }
}

static bool any_button_pressed() {
    return btn_mode.pressed || btn_up.pressed || btn_down.pressed || btn_select.pressed;
}

static void read_buttons() {
    update_button(btn_mode);
    update_button(btn_up);
    update_button(btn_down);
    update_button(btn_select);
}

static uint8_t days_in_month(uint8_t month, int year) {
    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap ? 29 : 28;
    }

    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }

    return 31;
}

static void clamp_date() {
    uint8_t max_day = days_in_month(ctx.current_time.month, ctx.current_time.year);

    if (ctx.current_time.day < 1) ctx.current_time.day = 1;
    if (ctx.current_time.day > max_day) ctx.current_time.day = max_day;
}

static void save_alarm() {
    EEPROM.update(EEPROM_ALARM_HOURS, ctx.alarm_hours);
    EEPROM.update(EEPROM_ALARM_MINUTES, ctx.alarm_minutes);
    EEPROM.update(EEPROM_ALARM_ENABLED, ctx.is_alarm_enabled ? 1 : 0);
}

static void clear_and_switch(enum screen next_screen) {
    if (current_screen != next_screen) {
        ctx.is_editing = false;
        ctx.edit_mode_index = 0;
        current_screen = next_screen;
        screen_dirty = true;

        if (ctx.lcd != NULL) {
            ctx.lcd->clear();
        }
    }
}

static void next_screen() {
    switch (current_screen) {
        case CLOCK_SCR:
            clear_and_switch(SHOW_DATE_SCR);
            break;
        case SHOW_DATE_SCR:
            clear_and_switch(SHOW_ENV_SCR);
            break;
        case SHOW_ENV_SCR:
            clear_and_switch(ALARM_SCR);
            break;
        case ALARM_SCR:
            clear_and_switch(CLOCK_SCR);
            break;
        default:
            clear_and_switch(CLOCK_SCR);
            break;
    }
}

static void adjust_current_datetime(int delta) {
    switch (ctx.edit_mode_index) {
        case 0:
            ctx.current_time.hours = (ctx.current_time.hours + 24 + delta) % 24;
            break;
        case 1:
            ctx.current_time.minutes = (ctx.current_time.minutes + 60 + delta) % 60;
            break;
        case 2: {
            uint8_t max_day = days_in_month(ctx.current_time.month, ctx.current_time.year);
            ctx.current_time.day = ((ctx.current_time.day - 1 + max_day + delta) % max_day) + 1;
            break;
        }
        case 3:
            ctx.current_time.month = ((ctx.current_time.month - 1 + 12 + delta) % 12) + 1;
            clamp_date();
            break;
        case 4:
            ctx.current_time.year += delta;
            if (ctx.current_time.year < 2000) ctx.current_time.year = 2099;
            if (ctx.current_time.year > 2099) ctx.current_time.year = 2000;
            clamp_date();
            break;
    }
}

static void adjust_alarm(int delta) {
    switch (ctx.edit_mode_index) {
        case 0:
            ctx.alarm_hours = (ctx.alarm_hours + 24 + delta) % 24;
            break;
        case 1:
            ctx.alarm_minutes = (ctx.alarm_minutes + 60 + delta) % 60;
            break;
        case 2:
            ctx.is_alarm_enabled = !ctx.is_alarm_enabled;
            break;
    }
}

static void start_editing() {
    ctx.is_editing = true;
    ctx.edit_mode_index = 0;
    screen_dirty = true;

    if (ctx.lcd != NULL) {
        ctx.lcd->clear();
    }
}

static void finish_datetime_edit_if_needed() {
    if (ctx.edit_mode_index <= 4) return;

    set_date(ctx.current_time.day, ctx.current_time.month, ctx.current_time.year);
    set_time(ctx.current_time.hours, ctx.current_time.minutes, 0);
    ctx.current_time = now();
    ctx.is_editing = false;
    ctx.edit_mode_index = 0;
    screen_dirty = true;

    if (ctx.lcd != NULL) {
        ctx.lcd->clear();
    }
}

static void finish_alarm_edit_if_needed() {
    if (ctx.edit_mode_index <= 2) return;

    save_alarm();
    ctx.is_editing = false;
    ctx.edit_mode_index = 0;
    screen_dirty = true;

    if (ctx.lcd != NULL) {
        ctx.lcd->clear();
    }
}

static void handle_editing() {
    if (btn_up.pressed) {
        if (current_screen == CLOCK_SCR) adjust_current_datetime(1);
        if (current_screen == ALARM_SCR) adjust_alarm(1);
        screen_dirty = true;
    }

    if (btn_down.pressed) {
        if (current_screen == CLOCK_SCR) adjust_current_datetime(-1);
        if (current_screen == ALARM_SCR) adjust_alarm(-1);
        screen_dirty = true;
    }

    if (btn_select.pressed) {
        ctx.edit_mode_index++;

        if (current_screen == CLOCK_SCR) {
            finish_datetime_edit_if_needed();
        } else if (current_screen == ALARM_SCR) {
            finish_alarm_edit_if_needed();
        }

        screen_dirty = true;
    }
}

static void handle_navigation() {
    if (ctx.is_editing) {
        handle_editing();
        return;
    }

    if (btn_mode.pressed) {
        next_screen();
        return;
    }

    if (btn_select.pressed && (current_screen == CLOCK_SCR || current_screen == ALARM_SCR)) {
        start_editing();
        return;
    }

    if ((btn_up.pressed || btn_down.pressed) && current_screen == ALARM_SCR) {
        ctx.is_alarm_enabled = !ctx.is_alarm_enabled;
        save_alarm();
        screen_dirty = true;
    }
}

static void start_alarm() {
    ctx.is_ringing = true;
    ctx.ring_started_at = millis();
    screen_dirty = true;

    if (ctx.lcd != NULL) {
        ctx.lcd->clear();
    }
}

static void stop_alarm() {
    ctx.is_ringing = false;
    noTone(BUZZER_PIN);
    digitalWrite(RGB_RED_PIN, LOW);
    digitalWrite(RGB_GREEN_PIN, HIGH);
    screen_dirty = true;

    if (ctx.lcd != NULL) {
        ctx.lcd->clear();
    }
}

static void update_alarm_state() {
    if (ctx.is_alarm_enabled && !ctx.is_ringing) {
        long key = ((long)ctx.current_time.year * 372L + (long)ctx.current_time.month * 31L + ctx.current_time.day) * 1440L
                   + (long)ctx.current_time.hours * 60L + ctx.current_time.minutes;

        if (ctx.current_time.hours == ctx.alarm_hours &&
            ctx.current_time.minutes == ctx.alarm_minutes &&
            key != last_alarm_key) {
            last_alarm_key = key;
            start_alarm();
        }
    }

    if (!ctx.is_ringing) return;

    if (any_button_pressed() || millis() - ctx.ring_started_at > ALARM_RING_TIMEOUT_MS) {
        stop_alarm();
        return;
    }

    digitalWrite(RGB_GREEN_PIN, LOW);
    digitalWrite(RGB_RED_PIN, (millis() / 250) % 2 ? HIGH : LOW);

    if ((millis() / 250) % 2) {
        tone(BUZZER_PIN, 1800);
    } else {
        tone(BUZZER_PIN, 1200);
    }
}

static void handle_factory_reset_combo() {
    bool combo_down = is_down(btn_mode) && is_down(btn_select);

    if (!combo_down) {
        reset_hold_started = 0;
        return;
    }

    if (reset_hold_started == 0) {
        reset_hold_started = millis();
    }

    if (millis() - reset_hold_started >= FACTORY_RESET_INTERVAL) {
        current_screen = FACTORY_RESET_SCR;
        reset_hold_started = 0;
        screen_dirty = true;
    }
}

static void render_screen() {
    enum screen next = current_screen;

    switch (current_screen) {
        case INIT_SCR:
            next = init_screen(&ctx);
            break;
        case CLOCK_SCR:
            next = clock_screen(&ctx);
            break;
        case SHOW_DATE_SCR:
            next = show_date_screen(&ctx);
            break;
        case SHOW_ENV_SCR:
            next = show_env_screen(&ctx);
            break;
        case ALARM_SCR:
            next = alarm_screen(&ctx);
            break;
        case FACTORY_RESET_SCR:
            next = factory_reset_screen(&ctx);
            break;
    }

    if (next != current_screen) {
        clear_and_switch(next);
    }
}

void setup() {
    pinMode(BTN1_PIN, INPUT_PULLUP);
    pinMode(BTN2_PIN, INPUT_PULLUP);
    pinMode(BTN3_PIN, INPUT_PULLUP);
    pinMode(BTN4_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);

    digitalWrite(RGB_RED_PIN, LOW);
    digitalWrite(RGB_GREEN_PIN, LOW);
    noTone(BUZZER_PIN);

    current_screen = INIT_SCR;
}

void loop() {
    read_buttons();

    if (!(ctx.is_editing && current_screen == CLOCK_SCR) && current_screen != INIT_SCR) {
        ctx.current_time = now();
    }

    handle_factory_reset_combo();
    update_alarm_state();

    if (!ctx.is_ringing && current_screen != FACTORY_RESET_SCR) {
        handle_navigation();
    }

    render_screen();
    delay(50);
}
