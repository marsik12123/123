#include "screens.h"
#include "context.h"
#include <stdio.h>

enum screen alarm_screen(struct context *context) {
    if (context == NULL || context->lcd == NULL) return ALARM_SCR;

    char line1[17];
    if (context->is_editing) {
        const char *field = "on/off";
        if (context->edit_mode_index == 0) field = "hours";
        if (context->edit_mode_index == 1) field = "minutes";
        snprintf(line1, sizeof(line1), "Edit %-10s", field);
    } else {
        snprintf(line1, sizeof(line1), "Alarm clock     ");
    }

    char line2[17];
    snprintf(line2, sizeof(line2), "Time %02u:%02u  %s ",
             context->alarm_hours, 
             context->alarm_minutes,
             context->is_alarm_enabled ? "ON " : "OFF");

    context->lcd->setCursor(0, 0);
    context->lcd->print(line1);
    context->lcd->setCursor(0, 1);
    context->lcd->print(line2);

    return ALARM_SCR;
}
