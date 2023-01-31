#include <OneWire.h>
#include <DS18B20.h>

#define COLD_SENSOR_PIN 32
#define HOT_SENSOR_PIN  25

OneWire coldSensorPin(COLD_SENSOR_PIN);
OneWire hotSensorPin(HOT_SENSOR_PIN);

DS18B20 coldSensor(&coldSensorPin);
DS18B20 hotSensor(&hotSensorPin);

#define TIME_OUT_MS 800UL

static bool setupTemperatureSensor(DS18B20& sensor) {
    if (!sensor.begin()) { return false; }
    sensor.setResolution(12);
    return true;
}

static float getTemperature(DS18B20& sensor) {
  uint32_t start = millis();
  while (!sensor.isConversionComplete()) {
      if (millis() - start >= TIME_OUT_MS) {
        // timeout
        Serial.printf("getTemperature: timeout\n");
          return __FLT_MIN__;
      }
  }
  return sensor.getTempC();
}

bool setupTemperatureSensors(void)
{
    if (!setupTemperatureSensor(coldSensor)) { return false; }
    if (!setupTemperatureSensor(hotSensor)) { return false; }
    return true;
}

void requestColdTemperature() {
    coldSensor.requestTemperatures();
}

void requestHotTemperature() {
    hotSensor.requestTemperatures();
}

float getColdTemperature() {
    float t = getTemperature(coldSensor);
    Serial.printf("getColdTemperature %f\n", t);
    return t;
}

float getHotTemperature() {
    float t = getTemperature(hotSensor);
    Serial.printf("getHotTemperature %f\n", t);
    return t;
}
