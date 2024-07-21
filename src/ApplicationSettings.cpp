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
        settings_.putBool(COLD_SENSOR_PRESENCE, false);
        settings_.putBool(HOT_SENSOR_PRESENCE, false);
        settings_.putBool(VOLTAGE_SENSOR_PRESENCE, false);
        settings_.putBool("global_init", true);
    } else {
        coldSensorPresence_ = settings_.getBool(COLD_SENSOR_PRESENCE);
        hotTSensorPresence_ = settings_.getBool(HOT_SENSOR_PRESENCE);
        voltageSensorPresence_ = settings_.getBool(VOLTAGE_SENSOR_PRESENCE);
    }
    settings_.end();
}

String ApplicationSettings::getJSON() {
    char jsonString[256];
    snprintf(jsonString, sizeof(jsonString), 
        "{\"pageSize\":%d,\"powerThreshold\":%d,\"coldSensorPresence\":%d,\"hotSensorPresence\":%d,\"voltageSensorPresence\":%d}",
        this->getPageSize(),
        this->getPowerThreshold(),
        this->getColdSensorPresence() ? 1 : 0,
        this->getHotSensorPresence() ? 1 : 0,
        this->getVoltageSensorPresence() ? 1 : 0
    );
    return String(jsonString);
}

bool ApplicationSettings::parseJSON(char *jsonTxt) {

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, (const char*)jsonTxt);
    if (error == DeserializationError::Ok) {
        if (doc.containsKey("pageSize")) this->setPageSize(doc["pageSize"]);
        if (doc.containsKey("powerThreshold")) this->setPowerThreshold(doc["powerThreshold"]);
        if (doc.containsKey("coldSensorPresence")) this->setColdSensorPresence(doc["coldSensorPresence"] == 1);
        if (doc.containsKey("hotSensorPresence")) this->setHotSensorPresence(doc["hotSensorPresence"] == 1);
        if (doc.containsKey("voltageSensorPresence")) this->setVoltageSensorPresence(doc["voltageSensorPresence"] == 1);
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
