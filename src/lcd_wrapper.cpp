#include "lcd_wrapper.h"
#include "config.h"
#include <Wire.h>

static I2C_LCD lcd(LCD_I2C_ADDRESS);

I2C_LCD *lcd_get() {
    return &lcd;
}

void lcd_init() {
    Wire.begin();
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.backlight();
    lcd.clear();
}

void lcd_clear() {
    lcd.clear();
}

void lcd_set_cursor(int y, int x) {
    lcd.setCursor((uint8_t)x, (uint8_t)y);
}

void lcd_print(const char* text) {
    lcd.print(text);
}

void lcd_print_at(int y, int x, const char* text) {
    lcd_set_cursor(y, x);
    lcd_print(text);
}

void lcd_backlight(bool state) {
    lcd.setBacklight(state);
}
