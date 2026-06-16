#include "sensors.h"
#include "config.h"
#include <Arduino.h>
#include <DHT.h>
#include <math.h>

static DHT dht(DHT_PIN, DHT_TYPE);
static float last_temperature = 0.0f;
static int last_humidity = 0;

void sensors_init() {
    dht.begin();
}

float get_temperature() {
    float value = dht.readTemperature();

    if (!isnan(value)) {
        last_temperature = value;
    }

    return last_temperature;
}

int get_humidity() {
    float value = dht.readHumidity();

    if (!isnan(value)) {
        last_humidity = (int)(value + 0.5f);
    }

    return last_humidity;
}
