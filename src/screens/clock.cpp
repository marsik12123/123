#include "screens.h"
#include "rtc_wrapper.h"
#include "sensors.h"
#include "config.h"
#include <stdio.h>

enum screen clock_screen(struct context *context) {
    if (context == NULL || context->lcd == NULL) return CLOCK_SCR;

    if (!context->is_editing) {
        context->current_time = now();
    }

    static unsigned long last_sensors_time = 0;
    unsigned long current_millis = millis();
    if (current_millis - last_sensors_time >= (SENSORS_READ_INTERVAL * 1000UL) || last_sensors_time == 0) {
        context->temperature = get_temperature();
        context->humidity = get_humidity();
        last_sensors_time = current_millis;
    }

    char line1[17];
    char line2[17];

    if (context->is_ringing) {
        snprintf(line1, sizeof(line1), "Wake up!        ");
        snprintf(line2, sizeof(line2), "Press any button");
    } else if (context->is_editing) {
        if (context->edit_mode_index < 2) {
            snprintf(line1, sizeof(line1), "Set time %02u:%02u ",
                     context->current_time.hours,
                     context->current_time.minutes);
            snprintf(line2, sizeof(line2), "Field: %s",
                     context->edit_mode_index == 0 ? "hours  " : "minutes");
        } else {
            snprintf(line1, sizeof(line1), "Set date        ");
            snprintf(line2, sizeof(line2), "%02u.%02u.%04d      ",
                     context->current_time.day,
                     context->current_time.month,
                     context->current_time.year);
        }
    } else {
        snprintf(line1, sizeof(line1), "%02u:%02u:%02u AL:%s",
                 context->current_time.hours,
                 context->current_time.minutes,
                 context->current_time.seconds,
                 context->is_alarm_enabled ? "ON " : "OFF");
        snprintf(line2, sizeof(line2), "%02u.%02u.%02d %2dC %2d%%",
                 context->current_time.day,
                 context->current_time.month,
                 context->current_time.year % 100,
                 (int)context->temperature,
                 (int)context->humidity);
    }

    context->lcd->setCursor(0, 0);
    context->lcd->print(line1);
    context->lcd->setCursor(0, 1);
    context->lcd->print(line2);

    return CLOCK_SCR;
}
