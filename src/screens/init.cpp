#include "screens.h"
#include "context.h"
#include "sensors.h"
#include "rtc_wrapper.h"
#include "lcd_wrapper.h"
#include "config.h"
#include <EEPROM.h>

enum screen init_screen(struct context *context) {
    if (context == NULL) return INIT_SCR;

    Serial.begin(BAUD_RATE);
    Serial.println("> Init Screen");

    context->lcd = lcd_get();

    sensors_init();
    clock_init();
    lcd_init();
    digitalWrite(RGB_RED_PIN, LOW);
    digitalWrite(RGB_GREEN_PIN, HIGH);

    context->alarm_hours = EEPROM.read(EEPROM_ALARM_HOURS);
    context->alarm_minutes = EEPROM.read(EEPROM_ALARM_MINUTES);
    context->is_alarm_enabled = (EEPROM.read(EEPROM_ALARM_ENABLED) == 1);
    
    if (context->alarm_hours > 23) context->alarm_hours = 0;
    if (context->alarm_minutes > 59) context->alarm_minutes = 0;

    context->is_ringing = false;
    context->is_editing = false;
    context->edit_mode_index = 0;
    context->ring_started_at = 0;

    context->temperature = get_temperature();
    context->humidity = get_humidity();
    context->current_time = now();

    return CLOCK_SCR;
}

enum screen factory_reset_screen(struct context *context) {
    if (context == NULL) return CLOCK_SCR;

    context->alarm_hours = 0;
    context->alarm_minutes = 0;
    context->is_alarm_enabled = false;
    context->is_ringing = false;
    context->is_editing = false;
    context->edit_mode_index = 0;

    EEPROM.update(EEPROM_ALARM_HOURS, context->alarm_hours);
    EEPROM.update(EEPROM_ALARM_MINUTES, context->alarm_minutes);
    EEPROM.update(EEPROM_ALARM_ENABLED, 0);

    noTone(BUZZER_PIN);
    digitalWrite(RGB_RED_PIN, LOW);
    digitalWrite(RGB_GREEN_PIN, HIGH);

    if (context->lcd != NULL) {
        context->lcd->clear();
        context->lcd->setCursor(0, 0);
        context->lcd->print("Factory reset  ");
        context->lcd->setCursor(0, 1);
        context->lcd->print("Alarm cleared   ");
    }

    delay(1200);
    if (context->lcd != NULL) context->lcd->clear();

    return CLOCK_SCR;
}
