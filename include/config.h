#ifndef CONFIG_H
#define CONFIG_H

/**
 * Configuration file for the alarm clock project.
 *
 * Feel free to modify the configuration file according to your needs. You can change all the values according to your needs except the name of the macros. If needed, you can add additional macros.
 */

// serial communication
#define BAUD_RATE 9600

// Wokwi has built-in DS1307 and DHT22 parts. Set this to 0 for a physical
// build with a DS1302 RTC module and a DHT11 sensor.
#define WOKWI_SIMULATION 1

// RTC Module
#define USE_DS1307_RTC WOKWI_SIMULATION
#define RTC_DAT_PIN 4
#define RTC_CLK_PIN 5
#define RTC_RST_PIN 9

// buzzer pin
#define BUZZER_PIN 6

// DHT sensor
#define DHT_PIN 8
#if WOKWI_SIMULATION
#define DHT_TYPE DHT22
#else
#define DHT_TYPE DHT11
#endif

// buttons
#define BTN1_PIN 7
#define BTN2_PIN 10
#define BTN3_PIN 11
#define BTN4_PIN 12

// status led pins
#define RGB_GREEN_PIN 14
#define RGB_RED_PIN 15

// I2C pins
#define SDA_PIN A4
#define SCL_PIN A5

// LCD Display
#define LCD_I2C_ADDRESS 0x27
#define LCD_ROWS 2
#define LCD_COLS 16

// interval for reading the values from sensors
#define SENSORS_READ_INTERVAL 1 * 60

// EEPROM layout
#define EEPROM_ALARM_HOURS 0
#define EEPROM_ALARM_MINUTES 1
#define EEPROM_ALARM_ENABLED 2

// button handling
#define BUTTON_DEBOUNCE_MS 40
#define ALARM_RING_TIMEOUT_MS 5 * 60 * 1000UL

// duration for entering the factory reset state
#define FACTORY_RESET_INTERVAL 3 * 1000

// for testing purposes
// #define __DATE__ "1.1.2000"
// #define __TIME__ "00:00:00"

#endif // CONFIG_H
