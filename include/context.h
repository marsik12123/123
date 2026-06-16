#ifndef CONTEXT_H
#define CONTEXT_H

#include <I2C_LCD.h>
#include "rtc_wrapper.h"

struct context {
    I2C_LCD *lcd;
    
    float temperature;
    float humidity;
    
    struct dt current_time;
    
    uint8_t alarm_hours;
    uint8_t alarm_minutes;
    bool is_alarm_enabled;
    bool is_ringing;
    bool is_editing;

    uint8_t edit_mode_index;
    unsigned long ring_started_at;
};

#endif // CONTEXT_H
