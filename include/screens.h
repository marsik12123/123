#ifndef SCREENS_H
#define SCREENS_H

#include "context.h"

enum screen {
    INIT_SCR,
    ALARM_SCR,
    CLOCK_SCR,
    FACTORY_RESET_SCR,
    SHOW_DATE_SCR,
    SHOW_ENV_SCR
};

enum screen init_screen(struct context *ctx);
enum screen clock_screen(struct context *ctx);
enum screen factory_reset_screen(struct context *ctx);
enum screen alarm_screen(struct context *ctx);
enum screen show_date_screen(struct context *ctx);
enum screen show_env_screen(struct context *ctx);

#endif // SCREENS_H 