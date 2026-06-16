#include "screens.h"
#include "context.h"
#include "sensors.h"
#include "config.h"
#include <stdio.h>

enum screen show_env_screen(struct context *context) {
    if (context == NULL || context->lcd == NULL) return SHOW_ENV_SCR;

    static unsigned long last_sensors_time = 0;
    unsigned long current_time = millis();
    if (current_time - last_sensors_time >= (SENSORS_READ_INTERVAL * 1000UL) || last_sensors_time == 0) {
        context->temperature = get_temperature();
        context->humidity = get_humidity();
        last_sensors_time = current_time;
    }

    char line1[17];
    snprintf(line1, sizeof(line1), "Temp: %5.1f C   ", (double)context->temperature);

    char line2[17];
    snprintf(line2, sizeof(line2), "Humidity: %3d%%  ", (int)context->humidity);

    context->lcd->setCursor(0, 0);
    context->lcd->print(line1);
    context->lcd->setCursor(0, 1);
    context->lcd->print(line2);

    return SHOW_ENV_SCR;
}
