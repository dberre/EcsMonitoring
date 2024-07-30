#include <Arduino.h>
#include <ArduinoJson.h>

#include "ApplicationSettings.h"

ApplicationSettings *ApplicationSettings::instance_ = 0;

ApplicationSettings *ApplicationSettings::instance() {
    if (instance_ == 0) {
        instance_ = new ApplicationSettings();
    }
    return instance_;
}

ApplicationSettings::ApplicationSettings()
{
    settings_.begin("global", false);
    if (settings_.isKey("global_init") == false) {
        Serial.println("Init global settings");
        settings_.putInt(PAGE_SIZE, 20);
        settings_.putInt(POWER_THRESHOLD, 15);
        settings_.putInt(TEMPERATURE_THRESHOLD, 10);
        settings_.putBool(COLD_SENSOR_PRESENCE, false);
        settings_.putBool(HOT_SENSOR_PRESENCE, false);
        settings_.putBool(VOLTAGE_SENSOR_PRESENCE, false);
        settings_.putInt(STORAGE_MODE, StorageMode::full);
        settings_.putBool("global_init", true);
    }

    coldSensorPresence_ = settings_.getBool(COLD_SENSOR_PRESENCE);
    hotTSensorPresence_ = settings_.getBool(HOT_SENSOR_PRESENCE);
    voltageSensorPresence_ = settings_.getBool(VOLTAGE_SENSOR_PRESENCE);
    temperatureThreshold_ = settings_.getInt(TEMPERATURE_THRESHOLD);
    storageMode_ = static_cast<StorageMode>(settings_.getInt(STORAGE_MODE));

    settings_.end();
}

String ApplicationSettings::getJSON() {
    char jsonString[256];
    snprintf(jsonString, sizeof(jsonString), 
        "{\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d}",
        PAGE_SIZE,
        this->getPageSize(),
        POWER_THRESHOLD,
        this->getPowerThreshold(),
        COLD_SENSOR_PRESENCE,
        this->getColdSensorPresence() ? 1 : 0,
        HOT_SENSOR_PRESENCE,
        this->getHotSensorPresence() ? 1 : 0,
        VOLTAGE_SENSOR_PRESENCE,
        this->getVoltageSensorPresence() ? 1 : 0,
        STORAGE_MODE,
        this->getStorageMode(),
        TEMPERATURE_THRESHOLD,
        this->getTemperatureThreshold()
    );
    return String(jsonString);
}

bool ApplicationSettings::parseJSON(char *jsonTxt) {

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, (const char*)jsonTxt);
    if (error == DeserializationError::Ok) {
        if (doc.containsKey(PAGE_SIZE)) this->setPageSize(doc[PAGE_SIZE]);
        if (doc.containsKey(POWER_THRESHOLD)) this->setPowerThreshold(doc[POWER_THRESHOLD]);
        if (doc.containsKey(COLD_SENSOR_PRESENCE)) this->setColdSensorPresence(doc[COLD_SENSOR_PRESENCE] == 1);
        if (doc.containsKey(HOT_SENSOR_PRESENCE)) this->setHotSensorPresence(doc[HOT_SENSOR_PRESENCE] == 1);
        if (doc.containsKey(VOLTAGE_SENSOR_PRESENCE)) this->setVoltageSensorPresence(doc[VOLTAGE_SENSOR_PRESENCE] == 1);
        if (doc.containsKey(STORAGE_MODE)) this->setStorageMode(doc[STORAGE_MODE]);
        if (doc.containsKey(TEMPERATURE_THRESHOLD)) this->setTemperatureThreshold(doc[TEMPERATURE_THRESHOLD]);
        return true;
    }
    return false;
}

int ApplicationSettings::getPageSize() {
    settings_.begin("global", true);
    int value = settings_.getInt(PAGE_SIZE);
    settings_.end();
    return value;
}

void ApplicationSettings::setPageSize(int size) {
    settings_.begin("global", false);
    settings_.putInt(PAGE_SIZE, size);
    settings_.end();
}

int ApplicationSettings::getPowerThreshold() {
    settings_.begin("global", true);
    int value = settings_.getInt(POWER_THRESHOLD);
    settings_.end();
    return value;
}

void ApplicationSettings::setPowerThreshold(int power) {
    settings_.begin("global", false);
    settings_.putInt(POWER_THRESHOLD, power);
    settings_.end();
}

int ApplicationSettings::getTemperatureThreshold() {
    return temperatureThreshold_;
}

void ApplicationSettings::setTemperatureThreshold(int temperature) {
    temperatureThreshold_ = temperature;
    settings_.begin("global", false);
    settings_.putBool(TEMPERATURE_THRESHOLD, temperature);
    settings_.end();
}

bool ApplicationSettings::getColdSensorPresence() {
    return coldSensorPresence_;
}

void ApplicationSettings::setColdSensorPresence(bool presence) {
    coldSensorPresence_ = presence;
    settings_.begin("global", false);
    settings_.putBool(COLD_SENSOR_PRESENCE, presence);
    settings_.end();
}

bool ApplicationSettings::getHotSensorPresence() {
    return hotTSensorPresence_;
}

void ApplicationSettings::setHotSensorPresence(bool presence) {
    hotTSensorPresence_ = presence;
    settings_.begin("global", false);
    settings_.putBool(HOT_SENSOR_PRESENCE, presence);
    settings_.end();
}

bool ApplicationSettings::getVoltageSensorPresence() {
    return voltageSensorPresence_;
}

void ApplicationSettings::setVoltageSensorPresence(bool presence) {
    voltageSensorPresence_ = presence;
    settings_.begin("global", false);
    settings_.putBool(VOLTAGE_SENSOR_PRESENCE, presence);
    settings_.end();
}

ApplicationSettings::StorageMode ApplicationSettings::getStorageMode() {
    return storageMode_;
}

void ApplicationSettings::setStorageMode(StorageMode mode) {
    storageMode_ = mode;
    settings_.begin("global", false);
    settings_.putBool(STORAGE_MODE, mode);
    settings_.end();
}
