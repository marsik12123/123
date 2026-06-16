#ifndef RTC_WRAPPER_H
#define RTC_WRAPPER_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dt {
    byte day;
    byte month;
    int year;
    byte hours;
    byte minutes;
    byte seconds;
};

void clock_init();
struct dt now();
void set_date(const byte day, const byte month, const int year);
void set_time(const byte hours, const byte minutes, const byte seconds);

#ifdef __cplusplus
}
#endif

#endif // RTC_WRAPPER_H