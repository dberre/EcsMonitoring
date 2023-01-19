#include <OneWire.h>
#include <DS18B20.h>

// numbers chosen to match pin numbers..
#define ONE_WIRE_BUS2               32
#define ONE_WIRE_BUS3               25

OneWire oneWire2(ONE_WIRE_BUS2);
OneWire oneWire3(ONE_WIRE_BUS3);

DS18B20 coldSensor(&oneWire2);
DS18B20 hotSensor(&oneWire3);

static bool setupTemperatureSensor(DS18B20& sensor) {
    if (!sensor.begin()) { return false; }
    sensor.setResolution(12);
    return true;
}

static float getTemperature(DS18B20& sensor) {
  uint32_t start = millis();
  uint32_t timeout = millis();
  while (!sensor.isConversionComplete()) {
      if (millis() - timeout >= 800) {
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

void requestTemperatures() {
    coldSensor.requestTemperatures();
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
