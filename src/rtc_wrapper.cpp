#include "rtc_wrapper.h"
#include "config.h"
#if USE_DS1307_RTC
#include <Wire.h>
#include <RtcDS1307.h>
#else
#include <RtcDS1302.h>
#endif

#if USE_DS1307_RTC
static RtcDS1307<TwoWire> rtc(Wire);
#else
static ThreeWire rtc_wire(RTC_DAT_PIN, RTC_CLK_PIN, RTC_RST_PIN);
static RtcDS1302<ThreeWire> rtc(rtc_wire);
#endif

static struct dt from_rtc(const RtcDateTime& value) {
    struct dt result;
    result.day = value.Day();
    result.month = value.Month();
    result.year = value.Year();
    result.hours = value.Hour();
    result.minutes = value.Minute();
    result.seconds = value.Second();
    return result;
}

static RtcDateTime to_rtc(byte day, byte month, int year, byte hours, byte minutes, byte seconds) {
    if (year < 2000) year = 2000;
    if (year > 2099) year = 2099;
    if (month < 1) month = 1;
    if (month > 12) month = 12;
    if (day < 1) day = 1;
    if (day > 31) day = 31;
    if (hours > 23) hours = 23;
    if (minutes > 59) minutes = 59;
    if (seconds > 59) seconds = 59;

    return RtcDateTime((uint16_t)year, month, day, hours, minutes, seconds);
}

void clock_init() {
    rtc.Begin();

#if !USE_DS1307_RTC
    if (rtc.GetIsWriteProtected()) {
        rtc.SetIsWriteProtected(false);
    }
#endif

    if (!rtc.GetIsRunning()) {
        rtc.SetIsRunning(true);
    }

    if (!rtc.IsDateTimeValid()) {
        rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
    }
}

struct dt now() {
    RtcDateTime current = rtc.GetDateTime();

    if (!current.IsValid()) {
        current = RtcDateTime(__DATE__, __TIME__);
    }

    return from_rtc(current);
}

void set_date(const byte day, const byte month, const int year) {
    struct dt current = now();
    rtc.SetDateTime(to_rtc(day, month, year, current.hours, current.minutes, current.seconds));
}

void set_time(const byte hours, const byte minutes, const byte seconds) {
    struct dt current = now();
    rtc.SetDateTime(to_rtc(current.day, current.month, current.year, hours, minutes, seconds));
}
