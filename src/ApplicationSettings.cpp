#include <Arduino.h>

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
        settings_.putBool("global_init", true);
    }
    settings_.end();
}

String ApplicationSettings::getJSON() {
    char jsonString[80];
    snprintf(jsonString, sizeof(jsonString), 
        "{\"pageSize\":\"%d\",\"powerThreshold\":\"%d\"}",
        this->getPageSize(),
        this->getPowerThreshold()
    );
    return String(jsonString);
}

int ApplicationSettings::getPageSize() {
    settings_.begin("global", true);
    int value = settings_.getInt(PAGE_SIZE);
    settings_.end();
    return value;
}

void ApplicationSettings::setPageSize(int size) {
    Serial.printf("setPageSize:%d\n", size);
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