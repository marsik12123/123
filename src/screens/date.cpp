#include "screens.h"
#include "context.h"
#include "rtc_wrapper.h"
#include <stdio.h>

enum screen show_date_screen(struct context *context) {
    if (context == NULL || context->lcd == NULL) return SHOW_DATE_SCR;

    if (!context->is_editing) {
        context->current_time = now();
    }

    char line1[17];
    snprintf(line1, sizeof(line1), "Current date    ");

    char line2[17];
    snprintf(line2, sizeof(line2), "  %02u.%02u.%04d   ",
             context->current_time.day, 
             context->current_time.month, 
             context->current_time.year);

    context->lcd->setCursor(0, 0);
    context->lcd->print(line1);
    context->lcd->setCursor(0, 1);
    context->lcd->print(line2);

    return SHOW_DATE_SCR;
}
